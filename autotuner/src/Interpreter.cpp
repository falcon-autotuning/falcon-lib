#include "falcon-autotuner/Interpreter.hpp"
#include "falcon-atc/AST.hpp"
#include "falcon_core/physics/device_structures/Connection.hpp"
#include "falcon_core/physics/device_structures/Connections.hpp"
#include <iostream>

namespace falcon::autotuner {

struct ToBool {
  bool operator()(bool b) const { return b; }
  bool operator()(int64_t i) const { return i != 0; }
  bool operator()(double d) const { return d != 0.0; }
  template <typename T> bool operator()(const T &) const { return false; }
};

Interpreter::Interpreter(
    const atc::Program &prog,
    const falcon_core::physics::config::core::Config &config,
    const std::string &nats_url)
    : program_(prog), config_(config) {
  init_nats(nats_url);
}

Interpreter::~Interpreter() { cleanup_nats(); }

void Interpreter::init_nats(const std::string &url) {
  natsStatus s = natsOptions_Create(&opts_);
  if (s == NATS_OK) {
    s = natsOptions_SetURL(opts_, url.c_str());
  }
  if (s == NATS_OK) {
    s = natsConnection_Connect(&conn_, opts_);
  }

  if (s != NATS_OK) {
    std::cerr << "Failed to connect to NATS: " << natsStatus_GetText(s)
              << std::endl;
  }
}

void Interpreter::cleanup_nats() {
  if (conn_) {
    natsConnection_Destroy(conn_);
    conn_ = nullptr;
  }
  if (opts_) {
    natsOptions_Destroy(opts_);
    opts_ = nullptr;
  }
}

bool Interpreter::run(const std::string &autotuner_name, ParameterMap &params) {
  auto at = find_autotuner(autotuner_name);
  if (!at) {
    std::cerr << "Autotuner not found: " << autotuner_name << std::endl;
    return false;
  }

  Context ctx;
  ctx.local_params = params;
  ctx.current_at = at;

  // Handle entry state
  std::string start_state = at->entry_state;
  if (start_state.empty()) {
    if (!at->states.empty())
      start_state = at->states[0].name;
    else if (!at->loops.empty() && !at->loops[0].states.empty())
      start_state = at->loops[0].states[0].name;
  }

  ctx.current_state = find_state(*at, start_state);

  while (ctx.current_state) {
    if (!execute_state(ctx))
      return false;
  }

  params = ctx.local_params;
  return true;
}

bool Interpreter::execute_state(Context &ctx) {
  if (!ctx.current_state)
    return false;

  ExprEvaluator eval(
      ctx.local_params, config_,
      [this](const std::string &name,
             const std::vector<ExprEvaluator::Value> &args)
          -> ExprEvaluator::Value {
        if (name == "spec_read") {
          if (args.empty())
            throw std::runtime_error("spec_read requires a name argument");
          std::string char_name = std::get<std::string>(args[0]);

          if (!conn_)
            return DeviceCharacteristic{}; // Mock or failure

          nlohmann::json req = {
              {"name", char_name},
              {"history_index", 0},
              {"hash", "interpreter_req_" + std::to_string(rand())},
              {"unitname", "interpreter"}};

          natsMsg *reply = nullptr;
          natsStatus s = natsConnection_RequestString(
              &reply, conn_, "READ_COMMAND", req.dump().c_str(), 1000);
          if (s == NATS_OK) {
            auto j = nlohmann::json::parse(natsMsg_GetData(reply));
            DeviceCharacteristic dc = DeviceCharacteristic::from_json(j);
            natsMsg_Destroy(reply);
            return dc;
          }
          return DeviceCharacteristic{};
        }

        if (name == "spec_load") {
          if (args.size() < 2)
            throw std::runtime_error(
                "spec_load requires name and characteristic arguments");

          std::string char_name = std::get<std::string>(args[0]);
          DeviceCharacteristic dc = std::get<DeviceCharacteristic>(args[1]);
          dc.name = char_name; // Ensure name matches

          if (conn_) {
            nlohmann::json load_cmd = dc.to_json();
            load_cmd["hash"] = "interpreter_load_" + std::to_string(rand());
            natsConnection_PublishString(conn_, "LOAD",
                                         load_cmd.dump().c_str());
          }
          return true;
        }

        throw std::runtime_error("Unknown built-in function: " + name);
      });

  // 1. Evaluate transitions
  for (const auto &t : ctx.current_state->transitions) {
    bool cond = true;
    if (t.condition) {
      auto val = eval.evaluate(t.condition.get());
      cond = std::visit(ToBool{}, val);
    }

    if (cond) {
      // Apply assignments
      for (const auto &asgn : t.assignments) {
        auto val = eval.evaluate(asgn.expression.get());
        for (const auto &target : asgn.targets) {
          ctx.local_params.set(target, val);
        }
      }

      // Check success/fail
      if (t.is_success || t.target.state_name == "_SUCCESS_") {
        // In a loop, success might mean "next item"
        if (t.target.parameter) {
          if (auto var = dynamic_cast<const atc::MemberExpr *>(
                  t.target.parameter.get())) {
            if (var->member == "next") {
              // Advance loop
              if (auto obj_var =
                      dynamic_cast<const atc::VarExpr *>(var->object.get())) {
                std::string loop_var = obj_var->name;
                auto &idx = ctx.loop_indices[loop_var];
                idx++;

                // Find the loop
                for (const auto &loop : ctx.current_at->loops) {
                  if (loop.variable == loop_var) {
                    auto it_val = eval.evaluate(loop.iterable.get());
                    if (auto conns = std::get_if<ConnectionsSP>(&it_val)) {
                      if (idx < (*conns)->size()) {
                        ctx.local_params.set(loop_var, (*conns)->at(idx));
                        ctx.current_state = &loop.states[0];
                        return true;
                      }
                    }
                  }
                }
              }
            }
          }
        }
        ctx.current_state = nullptr;
        return true;
      }

      if (t.is_fail || t.target.state_name == "_FAIL_") {
        ctx.current_state = nullptr;
        return false;
      }

      // Normal transition
      auto next_at = ctx.current_at;
      if (!t.target.autotuner_name.empty()) {
        next_at = find_autotuner(t.target.autotuner_name);
      }

      if (!next_at) {
        std::cerr << "Autotuner not found: " << t.target.autotuner_name
                  << std::endl;
        return false;
      }

      ctx.current_at = next_at;
      ctx.current_state = find_state(*next_at, t.target.state_name);
      return true;
    }
  }

  ctx.current_state = nullptr;
  return true;
}

const atc::AutotunerDecl *Interpreter::find_autotuner(const std::string &name) {
  for (const auto &at : program_.autotuners) {
    if (at.name == name)
      return &at;
  }
  return nullptr;
}

const atc::StateDecl *Interpreter::find_state(const atc::AutotunerDecl &at,
                                              const std::string &state_name) {
  for (const auto &s : at.states) {
    if (s.name == state_name)
      return &s;
  }
  for (const auto &loop : at.loops) {
    for (const auto &s : loop.states) {
      if (s.name == state_name)
        return &s;
    }
  }
  return nullptr;
}

} // namespace falcon::autotuner
