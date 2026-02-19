#include "falcon-autotuner/Interpreter.hpp"
#include "falcon-atc/AST.hpp"
#include "falcon-autotuner/ExprEvaluator.hpp"
#include "falcon-autotuner/log.hpp"
#include "falcon_core/physics/device_structures/Connection.hpp"
#include "falcon_core/physics/device_structures/Connections.hpp"
#include <falcon_core/communications/Time.hpp>
#include <falcon_core/physics/config/core/Config.hpp>
#include <fmt/format.h>
#include <iostream>
namespace {
const int INSTRUMENT_SERVER_LATENCY = 1000;
constexpr std::string_view DEVICE_SPECIFICATION_READ = "spec_read";
constexpr std::string_view DEVICE_SPECIFICATION_LOAD = "spec_load";
} // namespace
namespace falcon::autotuner {
using falcon_core::communications::Time;

struct ToBool {
  bool operator()(bool var) const { return var; }
  bool operator()(int64_t var) const { return var != 0; }
  bool operator()(double var) const { return var != 0.0; }
  template <typename T> bool operator()(const T & /*unused*/) const {
    return false;
  }
};

Interpreter::Interpreter(const atc::Program &prog) : program_(prog) {
  auto resp = comms_.subscribe_config_response(INSTRUMENT_SERVER_LATENCY,
                                               (int)Time().time());
  config_ = falcon_core::physics::config::core::Config::from_json_string<
      falcon_core::physics::config::core::Config>(resp.response);
}

Interpreter::~Interpreter() = default;

bool Interpreter::run(const std::string &autotuner_name, ParameterMap &params) {
  auto at = find_autotuner(autotuner_name);
  if (at == nullptr) {
    std::cerr << "Autotuner not found: " << autotuner_name << '\n';
    return false;
  }

  Context ctx;
  ctx.local_params = params;
  ctx.current_at = at;

  // Handle entry state
  std::string start_state = at->entry_state;
  if (start_state.empty()) {
    log::warn("No entry state specified, defaulting to first state");
    if (!at->states.empty()) {
      start_state = at->states[0].name;
    } else if (!at->loops.empty() && !at->loops[0].states.empty()) {
      start_state = at->loops[0].states[0].name;
    }
  }

  ctx.current_state = find_state(*at, start_state);

  evaluate_initial_params(ctx.local_params, *at);

  while (ctx.current_state != nullptr) {
    if (!execute_state(ctx)) {
      return false;
    }
  }

  params = ctx.local_params;
  return true;
}
ExprEvaluator::Value Interpreter::device_specification_read(
    const std::vector<ExprEvaluator::Value> &queries) {
  if (queries.empty()) {
    throw std::runtime_error("device-specification access requires args");
  }
  std::string char_name = std::get<std::string>(queries[0]);
  database::DeviceCharacteristicQuery query;
  query.scope = "device-specification";
  //  FIX: finish filling out the query
  if (query.name != char_name) {
    log::info(
        fmt::format("The characteristic name does not match the name in "
                    "the characteristic. Switching the name from {} to {}",
                    query.name.value_or(""), char_name));
  }
  query.name = char_name;
  auto matches = db_.get_by_query(query);
  //  FIX: either filter the results or return many chars
  //  FIX: currently returning the first match from the db
  return matches[0];
};
bool Interpreter::device_specification_load(
    const std::vector<ExprEvaluator::Value> &args) {
  if (args.empty()) {
    throw std::runtime_error("device-specification access requires args");
  }
  std::string char_name = std::get<std::string>(args[0]);
  if (args.size() < 2) {
    throw std::runtime_error(
        "spec_load requires name and characteristic arguments");
  }
  database::DeviceCharacteristic dchar =
      std::get<database::DeviceCharacteristic>(args[1]);
  dchar.name = char_name; // Ensure name matches
                          //  FIX: finish filling out the optional arguments
  db_.insert(dchar);
  return true;
}
void Interpreter::evaluate_initial_params(ParameterMap &params,
                                          const atc::AutotunerDecl &atuner) {
  auto eval_database = [this](const std::string &name,
                              const std::vector<ExprEvaluator::Value> &args)
      -> ExprEvaluator::Value {
    if (name != DEVICE_SPECIFICATION_READ) {
      throw std::runtime_error("Unknown built-in function: " + name);
    }
    if (args.empty()) {
      throw std::runtime_error("device-specification access requires args");
    }
    return device_specification_read(args);
  };
  ExprEvaluator eval(params, config_, eval_database);
  for (const auto &unevaluated_param : atuner.params) {
    auto val = eval.evaluate(unevaluated_param.default_value->clone());
    params.set(unevaluated_param.name, val);
  }
}
bool Interpreter::execute_state(Context &ctx) {
  auto eval_database = [this](const std::string &name,
                              const std::vector<ExprEvaluator::Value> &args)
      -> ExprEvaluator::Value {
    if (name != DEVICE_SPECIFICATION_READ &&
        name != DEVICE_SPECIFICATION_LOAD) {
      throw std::runtime_error("Unknown built-in function: " + name);
    }
    if (args.empty()) {
      throw std::runtime_error("device-specification access requires args");
    }
    std::string char_name = std::get<std::string>(args[0]);
    if (name == DEVICE_SPECIFICATION_READ) {
      return device_specification_read(args);
    }
    return device_specification_load(args);
  };

  if (ctx.current_state == nullptr) {
    return false;
  }
  ExprEvaluator eval(ctx.local_params, config_, eval_database);

  evaluate_state_params()
      // TODO: evaluate_temp_params()
      // 1. Evaluate transitions
      for (const auto &t : ctx.current_state->transitions) {
    bool cond = true;
    if (t.condition) {
      auto val = eval.evaluate(t.condition->clone());
      cond = std::visit(ToBool{}, val);
    }

    if (cond) {
      // Apply assignments
      for (const auto &asgn : t.assignments) {
        auto val = eval.evaluate(asgn.expression->clone());
        for (const auto &target : asgn.targets) {
          ctx.local_params.set(target, val);
        }
      }

      // Check success/fail
      if (t.is_success || t.target.state_name == "_SUCCESS_") {
        // In a loop, success might mean "next item"
        if (t.target.parameter) {
          if (const auto *var = dynamic_cast<const atc::MemberExpr *>(
                  t.target.parameter.get())) {
            if (var->member == "next") {
              // Advance loop
              if (const auto *obj_var =
                      dynamic_cast<const atc::VarExpr *>(var->object.get())) {
                std::string loop_var = obj_var->name;
                auto &idx = ctx.loop_indices[loop_var];
                idx++;

                // Find the loop
                for (const auto &loop : ctx.current_at->loops) {
                  if (loop.variable == loop_var) {
                    auto it_val = eval.evaluate(loop.iterable->clone());
                    if (auto *conns = std::get_if<ConnectionsSP>(&it_val)) {
                      if (idx < (*conns)->size()) {
                        ctx.local_params.set(loop_var, (*conns)->at(idx));
                        ctx.current_state = loop.states.data();
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
  for (const atc::AutotunerDecl &tuner : program_.autotuners) {
    if (tuner.name == name) {
      return &tuner;
    }
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
