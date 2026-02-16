#include "../include/falcon-atc/AST.hpp"
#include "parser.tab.hpp"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

using namespace falcon::atc;

extern Program *program_root;
extern FILE *yyin;
extern int yyparse();

std::string sanitize(const std::string &name) {
  std::string res = name;
  for (char &c : res) {
    if (!isalnum(c))
      c = '_';
  }
  return res;
}

std::string string_of_expr(const Expr *e) {
  if (!e)
    return "true";
  if (auto c = dynamic_cast<const ConstExpr *>(e)) {
    if (std::holds_alternative<int64_t>(c->value))
      return std::to_string(std::get<int64_t>(c->value));
    if (std::holds_alternative<double>(c->value))
      return std::to_string(std::get<double>(c->value));
    if (std::holds_alternative<bool>(c->value))
      return std::get<bool>(c->value) ? "true" : "false";
    if (std::holds_alternative<std::string>(c->value))
      return "\"" + std::get<std::string>(c->value) + "\"";
  }
  if (auto v = dynamic_cast<const VarExpr *>(e)) {
    if (v->name == "config")
      return "config";
    return "params.get<Value>(\"" + v->name + "\")";
  }
  if (auto m = dynamic_cast<const MemberExpr *>(e)) {
    if (auto obj_var = dynamic_cast<const VarExpr *>(m->object.get())) {
      if (obj_var->name == "config") {
        return "config." + m->member + "()";
      }
    }
    return string_of_expr(m->object.get()) + "." + m->member + "()";
  }
  if (auto b = dynamic_cast<const BinaryExpr *>(e)) {
    return "(" + string_of_expr(b->left.get()) + " " + b->op + " " +
           string_of_expr(b->right.get()) + ")";
  }
  if (auto u = dynamic_cast<const UnaryExpr *>(e)) {
    return u->op + "(" + string_of_expr(u->expr.get()) + ")";
  }
  if (auto call = dynamic_cast<const CallExpr *>(e)) {
    std::string res = call->name + "(";
    for (size_t i = 0; i < call->args.size(); ++i) {
      res += string_of_expr(call->args[i].get());
      if (i + 1 < call->args.size())
        res += ", ";
    }
    res += ")";
    return res;
  }
  return "true";
}

std::string escape_quotes(const std::string &s) {
  std::string res;
  for (char c : s) {
    if (c == '\"')
      res += "\\\"";
    else
      res += c;
  }
  return res;
}

void generate_code(const Program &prog, const std::string &output_prefix) {
  std::string h_code, s_code;

  auto write_h = [&](const std::string &line) { h_code += line + "\n"; };
  auto write_s = [&](const std::string &line) { s_code += line + "\n"; };

  write_h("// GENERATED CODE - DO NOT EDIT");
  write_h("#pragma once");
  write_h("");
  write_h("#include \"falcon-autotuner/Autotuner.hpp\"");
  write_h("#include <memory>");
  write_h("");
  write_h("namespace falcon::autotuner::generated {");
  write_h("");

  write_s("// GENERATED CODE - DO NOT EDIT");
  write_s("#include \"" + output_prefix + ".hpp\"");
  write_s("#include <iostream>");
  write_s("");
  write_s("namespace falcon::autotuner::generated {");
  write_s("");

  for (const auto &at : prog.autotuners) {
    std::string builder = at.name + "Builder";
    write_h("class " + builder + " {");
    write_h("public:");
    write_h("  " + builder + "();");
    write_h("  std::shared_ptr<Autotuner> get_autotuner() const { return "
            "autotuner_; }");
    write_h("private:");
    write_h("  void build_state_machine();");
    write_h("  std::shared_ptr<Autotuner> autotuner_;");
    write_h("};");
    write_h("");

    write_s(builder + "::" + builder + "() {");
    write_s("  autotuner_ = std::make_shared<Autotuner>(\"" + at.name + "\");");
    write_s("  build_state_machine();");
    write_s("}");
    write_s("");

    write_s("void " + builder + "::build_state_machine() {");

    for (const auto &s : at.states) {
      write_s("  auto state_" + sanitize(s.name) +
              " = autotuner_->create_state(\"" + s.name + "\");");
      if (s.measurement) {
        write_s("  state_" + sanitize(s.name) + "->set_measurement_command(\"" +
                escape_quotes(string_of_expr(s.measurement.get())) + "\");");
      }
      if (s.is_terminal) {
        write_s("  state_" + sanitize(s.name) + "->set_terminal(true);");
      }
    }

    for (const auto &loop : at.loops) {
      write_s("  // For loop: " + loop.variable + " in " +
              string_of_expr(loop.iterable.get()));
      for (const auto &s : loop.states) {
        write_s("  auto state_" + sanitize(s.name) +
                " = autotuner_->create_state(\"" + s.name + "\");");
        if (s.measurement) {
          write_s("  state_" + sanitize(s.name) +
                  "->set_measurement_command(\"" +
                  escape_quotes(string_of_expr(s.measurement.get())) + "\");");
        }
      }
    }

    write_s("");

    auto gen_at_transitions = [&](const std::vector<StateDecl> &vec_states) {
      for (const auto &s : vec_states) {
        std::string s_var = "state_" + sanitize(s.name);
        write_s("  // State: " + s.name);
        for (const auto &t : s.transitions) {
          std::string cond =
              t.condition ? string_of_expr(t.condition.get()) : "true";

          write_s("  " + s_var + "->add_transition(StateTransition(");

          if (t.is_success || t.target.state_name == "_SUCCESS_") {
            write_s("    StateId(\"\", \"_SUCCESS_\"),");
          } else if (t.is_fail || t.target.state_name == "_FAIL_") {
            write_s("    StateId(\"\", \"_FAIL_\"),");
          } else if (t.target.state_name == "_TERMINAL_") {
            write_s("    StateId(\"\", \"_TERMINAL_\"),");
          } else {
            write_s("    StateId(\"" + t.target.autotuner_name + "\", \"" +
                    t.target.state_name + "\"),");
          }

          write_s("    TransitionCondition([](const ParameterMap& params) {");
          for (const auto &asgn : t.assignments) {
            for (const auto &target : asgn.targets) {
              write_s("      // " + target + " = " +
                      string_of_expr(asgn.expression.get()));
            }
          }
          if (t.is_fail) {
            write_s("      // Error: " + t.error_message);
          }
          write_s("      return " + cond + ";");
          write_s("    }, \"" + escape_quotes(cond) + "\")");
          write_s("  ));");
        }
      }
    };

    gen_at_transitions(at.states);
    for (const auto &loop : at.loops)
      gen_at_transitions(loop.states);

    write_s("  autotuner_->set_entry_state(\"" + at.entry_state + "\");");
    write_s("}");
    write_s("");
  }

  write_h("} // namespace falcon::autotuner::generated");
  write_s("} // namespace falcon::autotuner::generated");

  std::ofstream h_out(output_prefix + ".hpp");
  if (h_out.is_open())
    h_out.write(h_code.data(), h_code.size());
  std::ofstream s_out(output_prefix + ".cpp");
  if (s_out.is_open())
    s_out.write(s_code.data(), s_code.size());
}

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <input.fal> [output_prefix]\n";
    return 1;
  }
  yyin = fopen(argv[1], "r");
  if (!yyin) {
    std::cerr << "Could not open input file: " << argv[1] << "\n";
    return 1;
  }
  if (yyparse() != 0)
    return 1;
  std::string output_prefix = (argc > 2) ? argv[2] : "GeneratedAutotuners";
  generate_code(*program_root, output_prefix);
  return 0;
}
