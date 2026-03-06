#include "falcon-dsl/ExprEvaluator.hpp"
#include "falcon-dsl/StmtExecutor.hpp"
#include "falcon-dsl/log.hpp"
#include <cmath>
#include <fmt/format.h>
#include <optional>
#include <sstream>

namespace falcon::dsl {
using fmt::format;
// ---------------------------------------------------------------------------
// Helper: substitute generic type params in a TypeDescriptor.
// subst maps param name (e.g. "T") → concrete TypeDescriptor.
// ---------------------------------------------------------------------------
static atc::TypeDescriptor
substitute_type(const atc::TypeDescriptor &td,
                const std::map<std::string, atc::TypeDescriptor> &subst) {
  if (td.is_struct()) {
    // If the struct_name is a known generic param, replace it.
    auto it = subst.find(td.struct_name);
    if (it != subst.end()) return it->second;
    // Otherwise keep as-is (plain struct or already-monomorphized name).
    return td;
  }
  if (td.is_array()) {
    if (td.element_type) {
      auto new_elem = substitute_type(*td.element_type, subst);
      return atc::TypeDescriptor::make_array(std::move(new_elem));
    }
  }
  // For primitive types (Int, Float, Bool, String) — no substitution needed.
  return td;
}

// ---------------------------------------------------------------------------
// Helper: build (or retrieve cached) monomorphized StructDecl.
//
// Given base_decl for "Box<T>" and type_args = [TypeDescriptor(Int)],
// creates a new StructDecl named "Box<int>" where every field / param
// whose type was "T" has been rewritten to "int".
//
// The result is registered in types_ so subsequent lookups find the cache.
// ---------------------------------------------------------------------------
static const atc::StructDecl *
monomorphize_struct(const atc::StructDecl &base_decl,
                    const std::vector<atc::TypeDescriptor> &type_args,
                    std::shared_ptr<falcon::dsl::TypeRegistry> types) {
  // Build monomorphized name, e.g. "Box<int>"
  std::string mono_name = base_decl.name + "<";
  for (size_t i = 0; i < type_args.size(); ++i) {
    if (i > 0) mono_name += ",";
    mono_name += type_args[i].to_string();
  }
  mono_name += ">";

  // Cache hit?
  if (types->has_monomorphized(mono_name)) {
    return types->lookup_struct(mono_name);
  }

  // Build substitution map: generic_params[i] → type_args[i]
  if (base_decl.generic_params.size() != type_args.size()) {
    throw falcon::dsl::EvaluationError(
        "Generic struct '" + base_decl.name + "' expects " +
        std::to_string(base_decl.generic_params.size()) +
        " type argument(s) but got " + std::to_string(type_args.size()));
  }
  std::map<std::string, atc::TypeDescriptor> subst;
  for (size_t i = 0; i < base_decl.generic_params.size(); ++i) {
    subst.emplace(base_decl.generic_params[i], type_args[i]);
  }

  // Clone fields with substituted types (no initializer cloning needed here;
  // initializers are re-evaluated at construction time anyway).
  std::vector<atc::VarDeclStmt> new_fields;
  for (const auto &f : base_decl.fields) {
    auto new_type = substitute_type(f.type, subst);
    // Clone initializer if present
    std::optional<std::unique_ptr<atc::Expr>> new_init = std::nullopt;
    if (f.initializer.has_value()) {
      new_init = f.initializer.value()->clone();
    }
    auto new_f = std::make_unique<atc::VarDeclStmt>(
        std::move(new_type), f.name, std::move(new_init));
    new_f->decl_scope = atc::VarDeclStmt::DeclScope::StructField;
    new_fields.push_back(std::move(*new_f));
  }

  // Clone routines with substituted param types.
  // (Bodies reference field names, not types, so they work as-is.)
  std::vector<atc::RoutineDecl> new_routines;
  for (const auto &r : base_decl.routines) {
    std::vector<std::unique_ptr<atc::ParamDecl>> new_ins, new_outs;
    for (const auto &p : r.input_params) {
      auto new_type = substitute_type(p->type, subst);
      new_ins.push_back(std::make_unique<atc::ParamDecl>(
          std::move(new_type), p->name));
    }
    for (const auto &p : r.output_params) {
      auto new_type = substitute_type(p->type, subst);

      if (new_type.is_struct()) {
        // Case 1: plain self-reference, e.g. output type is "Box" (no params).
        if (new_type.struct_name == base_decl.name) {
          new_type.struct_name = mono_name;
        }
        // Case 2: compound self-reference still containing generic param names,
        // e.g. output type is "Box<T>" or "Accumulator<T>".
        // substitute_type only replaces exact param-name keys (e.g. "T"),
        // so "Accumulator<T>" stays untouched. We detect this by checking
        // whether the struct_name starts with the base name followed by '<'.
        else if (new_type.struct_name.rfind(base_decl.name + "<", 0) == 0) {
          // Rebuild the full monomorphized name by substituting param names
          // inside the angle-bracket portion.
          // e.g. "Accumulator<T>" with subst {T→int} → "Accumulator<int>"
          std::string rebuilt = base_decl.name + "<";
          // Extract the comma-separated args inside the brackets.
          auto inner_start = new_type.struct_name.find('<') + 1;
          auto inner_end   = new_type.struct_name.rfind('>');
          std::string inner = new_type.struct_name.substr(
              inner_start, inner_end - inner_start);
          // Split on commas and substitute each token.
          std::vector<std::string> tokens;
          {
            std::string tok;
            for (char c : inner) {
              if (c == ',') { tokens.push_back(tok); tok.clear(); }
              else tok += c;
            }
            tokens.push_back(tok);
          }
          for (size_t ti = 0; ti < tokens.size(); ++ti) {
            if (ti > 0) rebuilt += ",";
            // Trim whitespace.
            auto &t = tokens[ti];
            t.erase(0, t.find_first_not_of(' '));
            t.erase(t.find_last_not_of(' ') + 1);
            auto sit = subst.find(t);
            if (sit != subst.end())
              rebuilt += sit->second.to_string();
            else
              rebuilt += t;
          }
          rebuilt += ">";
          new_type.struct_name = rebuilt;
        }
      }

      new_outs.push_back(std::make_unique<atc::ParamDecl>(
          std::move(new_type), p->name));
    }
    // Clone body stmts unchanged (field names stay the same)
    std::vector<std::unique_ptr<atc::Stmt>> new_body;
    for (const auto &s : r.body) new_body.push_back(s->clone());
    new_routines.push_back(atc::RoutineDecl(
        r.name, std::move(new_ins), std::move(new_outs), std::move(new_body)));
  }

  auto mono = std::make_unique<atc::StructDecl>(
      mono_name, std::vector<atc::VarDeclStmt>(std::move(new_fields)),
      std::vector<atc::RoutineDecl>(std::move(new_routines)));
  // No generic_params on the monomorphized copy.

  const atc::StructDecl *raw = mono.get();
  types->register_monomorphized_struct(mono_name, std::move(mono));
  return raw;
}
// ---------------------------------------------------------------------------
// Helper: strip the "Module::" prefix from a qualified name.
//   "Adder::adder"       → "adder"
//   "Quantity::Quantity"  → "Quantity"
//   "plain"              → "plain"
// ---------------------------------------------------------------------------
static std::string strip_module_prefix(const std::string &name) {
  auto pos = name.find("::");
  if (pos == std::string::npos)
    return name;
  return name.substr(pos + 2);
}

ExprEvaluator::ExprEvaluator(typing::ParameterMap &variables,
                             std::shared_ptr<FunctionRegistry> functions,
                             std::shared_ptr<TypeRegistry> types)
    : variables_(variables), functions_(std::move(functions)),
      types_(std::move(types)) {}

typing::RuntimeValue ExprEvaluator::evaluate(const atc::Expr &expr) {
  // Dispatch based on actual expression type
  if (const auto *lit = dynamic_cast<const atc::LiteralExpr *>(&expr)) {
    return eval_literal(*lit);
  }
  if (const auto *nil = dynamic_cast<const atc::NilLiteralExpr *>(&expr)) {
    return eval_nil_literal(*nil);
  }
  if (const auto *var = dynamic_cast<const atc::VarExpr *>(&expr)) {
    return eval_variable(*var);
  }
  if (const auto *bin = dynamic_cast<const atc::BinaryExpr *>(&expr)) {
    return eval_binary(*bin);
  }
  if (const auto *un = dynamic_cast<const atc::UnaryExpr *>(&expr)) {
    return eval_unary(*un);
  }
  if (const auto *mem = dynamic_cast<const atc::MemberExpr *>(&expr)) {
    return eval_member(*mem);
  }
  if (const auto *method = dynamic_cast<const atc::MethodCallExpr *>(&expr)) {
    return eval_method_call(*method);
  }
  if (const auto *idx = dynamic_cast<const atc::IndexExpr *>(&expr)) {
    return eval_index(*idx);
  }
  if (const auto *call = dynamic_cast<const atc::CallExpr *>(&expr)) {
    return eval_call(*call);
  }
  throw EvaluationError("Unknown expression type");
}

std::vector<typing::RuntimeValue> ExprEvaluator::evaluate_list(
    const std::vector<std::unique_ptr<atc::Expr>> &exprs) {
  std::vector<typing::RuntimeValue> results;
  results.reserve(exprs.size());
  for (const auto &expr : exprs) {
    results.push_back(evaluate(*expr));
  }
  return results;
}

typing::RuntimeValue ExprEvaluator::eval_literal(const atc::LiteralExpr &expr) {
  // LiteralExpr already stores the value in the correct variant format
  return std::visit(
      [](auto &&val) -> typing::RuntimeValue {
        return typing::RuntimeValue(val);
      },
      expr.value);
}

typing::RuntimeValue
ExprEvaluator::eval_nil_literal(const atc::NilLiteralExpr & /*expr*/) {
  return nullptr; // nil is represented as nullptr_t
}

typing::RuntimeValue ExprEvaluator::eval_variable(const atc::VarExpr &expr) {
  // Fast path: exact match in variables
  auto it = variables_.find(expr.name);
  if (it != variables_.end()) {
    return it->second;
  }

  // If the name is module-qualified ("Module::symbol"), try the bare symbol.
  // This handles cases where a qualified name slips through to variable lookup
  // (e.g. a type name like "Quantity::Quantity" used as a receiver before the
  // method-call static-receiver check gets a chance to catch it).
  if (expr.name.find("::") != std::string::npos) {
    auto bare = strip_module_prefix(expr.name);
    auto bare_it = variables_.find(bare);
    if (bare_it != variables_.end()) {
      return bare_it->second;
    }
  }

  throw EvaluationError("Undefined variable: " + expr.name);
}

typing::RuntimeValue ExprEvaluator::eval_binary(const atc::BinaryExpr &expr) {
  auto left = evaluate(*expr.left);
  auto right = evaluate(*expr.right);
  return apply_binary_op(expr.op, left, right);
}

typing::RuntimeValue ExprEvaluator::eval_unary(const atc::UnaryExpr &expr) {
  auto operand = evaluate(*expr.operand);
  return apply_unary_op(expr.op, operand);
}

typing::RuntimeValue ExprEvaluator::eval_member(const atc::MemberExpr &expr) {
  auto object = evaluate(*expr.object);

  // User-defined struct field access: q.a_
  if (std::holds_alternative<std::shared_ptr<typing::StructInstance>>(object)) {
    const auto &instancePtr =
        std::get<std::shared_ptr<typing::StructInstance>>(object);
    if (!instancePtr) {
      throw EvaluationError("StructInstance is nullptr");
    }
    return instancePtr->get_field(expr.member);
  }
  // TODO: falcon-core type field access (config.plunger_gates, etc.)
  throw EvaluationError("Member access not yet implemented for type '" +
                        get_runtime_type_name(object) + "': " + expr.member);
}

// ---------------------------------------------------------------------------
// Helper: execute one struct routine in its own sub-environment.
// ---------------------------------------------------------------------------
typing::FunctionResult ExprEvaluator::exec_struct_routine(
    std::shared_ptr<typing::StructInstance> receiver_instance,
    const atc::StructDecl &struct_decl, const atc::RoutineDecl &routine,
    const std::vector<typing::RuntimeValue> &call_args) {

  // ---- FFI fast-path -------------------------------------------------------
  // If the routine has no body AND an FFI method is registered, dispatch
  // through the C ABI. This covers two sub-cases:
  //
  //  A) Constructor / static call (receiver is a fresh, non-native instance):
  //     The FFI function allocates the native object and returns it wrapped
  //     in an opaque slot. We return that result directly — the caller
  //     (eval_method_call) stores it as the new variable value.
  //
  //  B) Instance method call (receiver IS a native FFI object,
  //  is_native()==true):
  //     Pack "this" from the native_handle and pass along with input args.
  //
  if (routine.body.empty()) {
    const typing::TypeMethod *ffi_method =
        types_->lookup_ffi_method(struct_decl.name, routine.name);
    // For monomorphized generic structs (e.g. "Box<int>"), the FFI methods
    // are registered under the base name ("Box").  Fall back to the base name
    // by stripping any "<...>" suffix.
    if (ffi_method == nullptr) {
      auto angle = struct_decl.name.find('<');
      if (angle != std::string::npos) {
        std::string base_name = struct_decl.name.substr(0, angle);
        ffi_method = types_->lookup_ffi_method(base_name, routine.name);
      }
    }

    if (ffi_method != nullptr) {
      typing::ParameterMap ffi_params;

      if (receiver_instance && receiver_instance->is_native()) {
        // Case B: instance method — forward the native handle as "this"
        log::debug("FFI dispatch (instance): " + struct_decl.name + "." +
                   routine.name);
        ffi_params["this"] = receiver_instance;
      } else {
        // Case A: constructor — no "this", only input args
        log::debug("FFI dispatch (constructor): " + struct_decl.name + "." +
                   routine.name);
      }

      // Pack input arguments by name from the routine signature
      for (size_t i = 0;
           i < routine.input_params.size() && i < call_args.size(); ++i) {
        ffi_params[routine.input_params[i]->name] = call_args[i];
      }

      typing::FunctionResult result =
          (*ffi_method)(receiver_instance, ffi_params);
      log::debug("FFI dispatch result count: " + std::to_string(result.size()));
      // Use runtime_value_to_string for safe serialization of all variant types
      for (size_t i = 0; i < result.size(); ++i) {
        log::debug("FFI dispatch result[" + std::to_string(i) +
                   "]: " + runtime_value_to_string(result[i]));
      }
      return result;
    }

    // Body is empty but no FFI method registered — always a hard error.
    throw EvaluationError("Struct routine '" + routine.name + "' on type '" +
                          struct_decl.name +
                          "' has an empty body and no FFI implementation");
  }

  // If body is empty but instance is NOT native, this is the silent failure
  // mode — the constructor returned nullptr or a plain StructInstance.
  if (routine.body.empty()) {
    log::debug("WARN: routine body empty but instance is not native: " +
               struct_decl.name + "." + routine.name + " is_native=" +
               std::to_string(receiver_instance ? receiver_instance->is_native()
                                                : -1));
  }

  // ---- Build the sub-environment ----------------------------------------
  typing::ParameterMap routine_env;

  // Bind "this" to the receiver so that:
  //   this.a   (MemberExpr on VarExpr("this"))
  // both work inside the routine body.
  routine_env["this"] = receiver_instance;

  // Flatten all struct fields into the routine environment so that bare
  // field names resolve directly without qualifying.
  for (const auto &field : struct_decl.fields) {
    auto field_it = receiver_instance->fields->find(field.name);
    if (field_it != receiver_instance->fields->end()) {
      routine_env[field.name] = field_it->second;
    } else if (field.initializer.has_value()) {
      ExprEvaluator sub_eval(routine_env, functions_, types_);
      routine_env[field.name] = sub_eval.evaluate(*field.initializer.value());
    } else {
      typing::RuntimeValue def;
      switch (field.type.base_type) {
      case atc::ParamType::Int:
        def = int64_t(0);
        break;
      case atc::ParamType::Float:
        def = 0.0;
        break;
      case atc::ParamType::Bool:
        def = false;
        break;
      case atc::ParamType::String:
        def = std::string("");
        break;
      default:
        def = nullptr;
        break;
      }
      routine_env[field.name] = def;
    }
  }

  // ---- Bind input parameters --------------------------------------------
  for (size_t i = 0; i < routine.input_params.size(); ++i) {
    if (i < call_args.size()) {
      routine_env[routine.input_params[i]->name] = call_args[i];
    } else if (routine.input_params[i]->default_value.has_value()) {
      ExprEvaluator sub_eval(routine_env, functions_, types_);
      routine_env[routine.input_params[i]->name] =
          sub_eval.evaluate(*routine.input_params[i]->default_value.value());
    } else {
      throw EvaluationError("Missing argument for struct routine '" +
                            routine.name +
                            "' parameter: " + routine.input_params[i]->name);
    }
  }

  // ---- Initialise output parameters -------------------------------------
  for (const auto &out : routine.output_params) {
    if (out->type.is_struct()) {
      auto fresh =
          std::make_shared<typing::StructInstance>(out->type.struct_name);
      const atc::StructDecl *out_decl =
          types_->lookup_struct(out->type.struct_name);
      if (out_decl) {
        ExprEvaluator sub_eval(routine_env, functions_, types_);
        for (const auto &field : out_decl->fields) {
          typing::RuntimeValue fval;
          if (field.initializer.has_value()) {
            fval = sub_eval.evaluate(*field.initializer.value());
          } else {
            switch (field.type.base_type) {
            case atc::ParamType::Int:
              fval = int64_t(0);
              break;
            case atc::ParamType::Float:
              fval = 0.0;
              break;
            case atc::ParamType::Bool:
              fval = false;
              break;
            case atc::ParamType::String:
              fval = std::string("");
              break;
            default:
              fval = nullptr;
              break;
            }
          }
          fresh->set_field(field.name, fval);
        }
      }
      routine_env[out->name] = fresh;
    } else if (out->default_value.has_value()) {
      ExprEvaluator sub_eval(routine_env, functions_, types_);
      routine_env[out->name] = sub_eval.evaluate(*out->default_value.value());
    }
  }

  // ---- Execute the routine body -----------------------------------------
  StmtExecutor sub_exec(routine_env, functions_, types_);
  sub_exec.execute_block(routine.body);
  // ---- Sync field variables FROM receiver_instance BACK INTO routine_env --
  // StructFieldAssignStmt mutations (e.g. "total = total + delta" inside a
  // struct routine) write directly to receiver_instance->fields via the "this"
  // pointer, bypassing routine_env entirely.  We must re-sync routine_env from
  // the instance BEFORE reading output params — otherwise output params that
  // read field values (e.g. "new_total = total") will see the stale pre-call
  // value, and the subsequent write-back will also clobber the mutations.
  for (const auto &field : struct_decl.fields) {
    auto field_it = receiver_instance->fields->find(field.name);
    if (field_it != receiver_instance->fields->end()) {
      routine_env[field.name] = field_it->second;
    }
  }

  // ---- Write flattened field variables back into receiver_instance -------
  for (const auto &field : struct_decl.fields) {
    auto it = routine_env.find(field.name);
    if (it != routine_env.end()) {
      receiver_instance->set_field(field.name, it->second);
    }
  }

  // ---- Collect and return output values ----------------------------------
  typing::FunctionResult outputs;
  for (const auto &out : routine.output_params) {
    auto it = routine_env.find(out->name);
    if (it == routine_env.end()) {
      throw EvaluationError("Struct routine '" + routine.name +
                            "' did not set output: " + out->name);
    }
    outputs.push_back(it->second);
  }
  return outputs;
}

typing::RuntimeValue
ExprEvaluator::eval_method_call(const atc::MethodCallExpr &expr) {
  // ------------------------------------------------------------------
  // Check first whether the object expression is a bare OR module-qualified
  // type name that refers to a known struct — i.e. a STATIC / CONSTRUCTOR
  // call like:
  //   Quantity.New(a)           → VarExpr("Quantity")
  //   Quantity::Quantity.New(a) → VarExpr("Quantity::Quantity")
  //
  // We must intercept this BEFORE evaluating the object as a variable,
  // because the type name is not a runtime variable.
  // ------------------------------------------------------------------
  if (const auto *type_var =
          dynamic_cast<const atc::VarExpr *>(expr.object.get())) {

    // Try the name as-is first (may already be "Box<int>"), then strip prefix.
    // Also try the base name extracted from inferred_type (set by exec_var_decl
    // when the LHS type is a generic instantiation, e.g. Box<int>).
    std::string lookup_name = type_var->name;

    // If a concrete mono name was injected via inferred_type, prefer it.
    // This handles:  Box<int> b = Box.New(x);
    //   → exec_var_decl sets inferred_type = TypeDescriptor for "Box<int>"
    //   → we look up "Box" to get the generic template, then monomorphize.
    std::vector<atc::TypeDescriptor> injected_type_args;
    if (expr.inferred_type.has_value() &&
        expr.inferred_type->is_generic_struct()) {
      // inferred_type.struct_name is e.g. "Box<int>" — extract base name.
      const std::string &mono = expr.inferred_type->struct_name;
      auto angle = mono.find('<');
      if (angle != std::string::npos)
        lookup_name = mono.substr(0, angle);
      injected_type_args = expr.inferred_type->type_args;
    }

    const atc::StructDecl *struct_decl = types_->lookup_struct(lookup_name);
    if (struct_decl == nullptr &&
          lookup_name.find("::") != std::string::npos) {
      struct_decl = types_->lookup_struct(strip_module_prefix(lookup_name));
    }

    if (struct_decl != nullptr &&
        variables_.find(type_var->name) == variables_.end() &&
        variables_.find(strip_module_prefix(type_var->name)) ==
            variables_.end()) {
      // It IS a struct type name used as a static receiver.

      // If this is a generic struct and we have injected type args, monomorphize
      // now so that constructor output params get the correct concrete type.
      if (struct_decl->is_generic() && !injected_type_args.empty()) {
        struct_decl = monomorphize_struct(*struct_decl, injected_type_args, types_);
      } else if (struct_decl->is_generic() && injected_type_args.empty()) {
        // Generic struct called without type args — this is a programmer error,
        // but we let it fall through; the routine will likely fail at output
        // param init time when it can't resolve the type param name.
        log::debug("WARN: generic struct '" + struct_decl->name +
                   "' used as constructor without type args in inferred_type");
      }

      const atc::RoutineDecl *routine =
          struct_decl->find_routine(expr.method_name);
      if (!routine) {
        throw EvaluationError("Unknown struct routine '" + expr.method_name +
                              "' on type '" + struct_decl->name + "'");
      }

      // Build a fresh receiver instance using the (possibly monomorphized) name.
      auto receiver = std::make_shared<typing::StructInstance>(struct_decl->name);
      for (const auto &field : struct_decl->fields) {
        typing::RuntimeValue fval;
        if (field.initializer.has_value()) {
          ExprEvaluator tmp(variables_, functions_, types_);
          fval = tmp.evaluate(*field.initializer.value());
        } else {
          switch (field.type.base_type) {
          case atc::ParamType::Int:
            fval = int64_t(0);
            break;
          case atc::ParamType::Float:
            fval = 0.0;
            break;
          case atc::ParamType::Bool:
            fval = false;
            break;
          case atc::ParamType::String:
            fval = std::string("");
            break;
          case atc::ParamType::Struct: {
            // Initialise a fresh empty instance; the FFI constructor will
            // replace it.
            fval = std::make_shared<typing::StructInstance>(
                field.type.struct_name);
            break;
          }
          default:
            fval = nullptr;
            break;
          }
        }
        receiver->set_field(field.name, fval);
      }

      std::vector<typing::RuntimeValue> call_args;
      call_args.reserve(expr.args.size());
      for (const auto &arg : expr.args) {
        call_args.push_back(evaluate(*arg));
      }

      typing::FunctionResult outputs =
          exec_struct_routine(receiver, *struct_decl, *routine, call_args);

      // DEBUG: log what the constructor produced
      log::debug("Constructor outputs.size(): " +
                 std::to_string(outputs.size()));
      if (!outputs.empty()) {
        log::debug("Constructor output[0] type: " +
                   get_runtime_type_name(outputs[0]));
        log::debug("Constructor output[0] value: " +
                   runtime_value_to_string(outputs[0]));
      }

      if (outputs.empty()) {
        return nullptr;
      }
      if (outputs.size() == 1) {
        return outputs[0];
      }
      return std::make_shared<typing::TupleValue>(outputs);
    }
  }

  // ------------------------------------------------------------------
  // Normal instance method call: q.Value(), q.ValueWithB(), etc.
  // ------------------------------------------------------------------
  auto object = evaluate(*expr.object);

  // User-defined struct instance method call
  if (std::holds_alternative<std::shared_ptr<typing::StructInstance>>(object)) {
    const auto &instancePtr =
        std::get<std::shared_ptr<typing::StructInstance>>(object);
    if (!instancePtr) {
      throw EvaluationError("StructInstance is nullptr");
    }

    const atc::StructDecl *struct_decl =
        types_->lookup_struct(instancePtr->type_name);
    if (!struct_decl) {
      throw EvaluationError("No struct declaration found for type: " +
                            instancePtr->type_name);
    }
    const atc::RoutineDecl *routine =
        struct_decl->find_routine(expr.method_name);
    if (!routine) {
      throw EvaluationError("Unknown struct routine '" + expr.method_name +
                            "' on type '" + instancePtr->type_name + "'");
    }

    std::vector<typing::RuntimeValue> call_args;
    call_args.reserve(expr.args.size());
    for (const auto &arg : expr.args) {
      call_args.push_back(evaluate(*arg));
    }

    typing::FunctionResult outputs =
        exec_struct_routine(instancePtr, *struct_decl, *routine, call_args);

    if (outputs.empty()) {
      return nullptr;
    }
    if (outputs.size() == 1) {
      return outputs[0];
    }
    return std::make_shared<typing::TupleValue>(outputs);
  }

  // ------------------------------------------------------------------
  // Existing falcon-core built-in type method dispatch.
  // Array methods are now fully handled by the Array<T> FFI library
  // (libs/collections/array) as regular StructInstance method calls.
  // The legacy ArrayValue dispatch below is retained only for
  // falconCore wrappers that pass raw ArrayValue objects through the FFI.
  // ------------------------------------------------------------------
  std::string type_name = get_runtime_type_name(object);

  // Legacy ArrayValue method dispatch (falconCore internal use only).
  // New code should use Array<T> StructInstance methods instead.
  auto *method = types_->lookup_method(type_name, expr.method_name);
  if (method == nullptr) {
    throw EvaluationError("Unknown method '" + expr.method_name +
                          "' for type '" + type_name + "'");
  }

  // Evaluate all positional arguments
  std::vector<typing::RuntimeValue> arg_values;
  arg_values.reserve(expr.args.size());
  for (const auto &arg : expr.args) {
    arg_values.push_back(evaluate(*arg));
  }

  // Map positional arguments to parameter names (position-based for legacy).
  typing::ParameterMap params;
  for (size_t i = 0; i < arg_values.size(); ++i) {
    params["arg" + std::to_string(i)] = arg_values[i];
  }

  auto result = (*method)(object, params);
  if (result.empty()) {
    return nullptr;
  }
  if (result.size() == 1) {
    return result[0];
  }
  std::vector<typing::RuntimeValue> values;
  values.reserve(result.size());
  for (const auto &val : result) {
    values.push_back(val);
  }
  return std::make_shared<typing::TupleValue>(values);
}

typing::RuntimeValue ExprEvaluator::eval_index(const atc::IndexExpr &expr) {
  auto object = evaluate(*expr.object);
  auto index  = evaluate(*expr.index);

  // ---------------------------------------------------------------------------
  // Array indexing: arr[i]  →  arr.GetIndex(i)
  //
  // Array<T> is now an FFI-backed StructInstance.  We route arr[i] through the
  // normal struct method-call path by synthesising a GetIndex call.  This
  // keeps the AST IndexExpr node (for parser compatibility) while delegating
  // all logic to the library-defined GetIndex routine.
  // ---------------------------------------------------------------------------
  if (std::holds_alternative<std::shared_ptr<typing::StructInstance>>(object)) {
    const auto &instPtr =
        std::get<std::shared_ptr<typing::StructInstance>>(object);
    if (!instPtr) {
      throw EvaluationError("Cannot index into a nil struct");
    }
    // Look up GetIndex on the struct's type.
    const atc::StructDecl *struct_decl =
        types_->lookup_struct(instPtr->type_name);
    if (struct_decl) {
      const atc::RoutineDecl *get_index = struct_decl->find_routine("GetIndex");
      if (get_index) {
        typing::FunctionResult outputs =
            exec_struct_routine(instPtr, *struct_decl, *get_index, {index});
        if (outputs.empty()) return nullptr;
        if (outputs.size() == 1) return outputs[0];
        return std::make_shared<typing::TupleValue>(outputs);
      }
    }
    // Also try lookup via FFI method (for monomorphized names like "Array<int>")
    const typing::TypeMethod *ffi_method =
        types_->lookup_ffi_method(instPtr->type_name, "GetIndex");
    if (ffi_method == nullptr) {
      // Try base name (strip "<...>")
      auto angle = instPtr->type_name.find('<');
      if (angle != std::string::npos) {
        std::string base = instPtr->type_name.substr(0, angle);
        ffi_method = types_->lookup_ffi_method(base, "GetIndex");
      }
    }
    if (ffi_method != nullptr) {
      typing::ParameterMap params;
      params["this"]  = object;
      params["index"] = index;
      auto result = (*ffi_method)(object, params);
      if (result.empty()) return nullptr;
      if (result.size() == 1) return result[0];
      return std::make_shared<typing::TupleValue>(result);
    }
    throw EvaluationError("Type '" + instPtr->type_name +
                          "' does not support indexing (no GetIndex routine)");
  }

  // Legacy: direct ArrayValue indexing (for falconCore internal use).
  if (std::holds_alternative<std::shared_ptr<typing::ArrayValue>>(object)) {
    const auto &arrPtr = std::get<std::shared_ptr<typing::ArrayValue>>(object);
    if (!arrPtr) {
      throw EvaluationError("Cannot index into a nil Array");
    }
    if (!std::holds_alternative<int64_t>(index)) {
      throw EvaluationError("Array index must be an integer, got: " +
                            get_runtime_type_name(index));
    }
    int64_t idx = std::get<int64_t>(index);
    if (idx < 0) {
      throw EvaluationError("Array index must be non-negative, got: " +
                            std::to_string(idx));
    }
    if (static_cast<size_t>(idx) >= arrPtr->size()) {
      throw EvaluationError(
          "Array index out of bounds: " + std::to_string(idx) +
          " (size=" + std::to_string(arrPtr->size()) + ")");
    }
    return (*arrPtr)[static_cast<size_t>(idx)];
  }

  throw EvaluationError("Indexing not supported for type: " +
                        get_runtime_type_name(object));
}

typing::RuntimeValue ExprEvaluator::eval_call(const atc::CallExpr &expr) {
  log::debug(fmt::format("Calling the func {}", expr.name));

  // Look up function — try the fully-qualified name first, then fall back to
  // the bare name (stripping any "Module::" prefix). This allows calls like
  // `Adder::adder(a, b)` to resolve when the routine was registered as "adder".
  auto *func = functions_->lookup(expr.name);
  std::string resolved_name = expr.name;
  if (func == nullptr && expr.name.find("::") != std::string::npos) {
    resolved_name = strip_module_prefix(expr.name);
    func = functions_->lookup(resolved_name);
  }
  if (func == nullptr) {
    throw EvaluationError("Unknown function: " + expr.name);
  }

  // Get signature — use the same resolved name
  const atc::BuiltinSignature *sig = functions_->get_signature(resolved_name);
  if (sig == nullptr) {
    throw EvaluationError("No signature found for function: " + expr.name);
  }
  log::debug("Found signature");
  log::debug(fmt::format("Searching for the arguments in the signature {}",
                         sig->qualified_name));
  log::debug(fmt::format("Searching for the arguments in the signature {}",
                         sig->supports_named_args));
  log::debug(fmt::format("Searching for the arguments in the signature {}",
                         sig->to_json().dump()));

  typing::ParameterMap params;
  std::vector<bool> assigned(expr.arguments.size(), false);

  // First: handle required parameters
  for (const auto &param_spec : sig->parameters) {
    if (!param_spec.required) {
      continue;
    }

    auto named_it = std::find_if(expr.arguments.begin(), expr.arguments.end(),
                                 [&](const atc::CallArg &arg) {
                                   return arg.name.has_value() &&
                                          arg.name.value() == param_spec.name;
                                 });

    if (named_it != expr.arguments.end()) {
      size_t idx = std::distance(expr.arguments.begin(), named_it);
      params[param_spec.name] = evaluate(*named_it->value);
      assigned[idx] = true;
      continue;
    }

    auto pos_it = std::find_if(expr.arguments.begin(), expr.arguments.end(),
                               [&](const atc::CallArg &arg) {
                                 size_t idx = &arg - expr.arguments.data();
                                 return !arg.name.has_value() && !assigned[idx];
                               });

    if (pos_it != expr.arguments.end()) {
      size_t idx = std::distance(expr.arguments.begin(), pos_it);
      params[param_spec.name] = evaluate(*pos_it->value);
      assigned[idx] = true;
      continue;
    }

    throw EvaluationError("Missing required argument: " + param_spec.name);
  }

  // Second: handle optional parameters
  for (const auto &param_spec : sig->parameters) {
    if (param_spec.required) {
      continue;
    }

    auto named_it = std::find_if(expr.arguments.begin(), expr.arguments.end(),
                                 [&](const atc::CallArg &arg) {
                                   return arg.name.has_value() &&
                                          arg.name.value() == param_spec.name;
                                 });

    if (named_it != expr.arguments.end()) {
      size_t idx = std::distance(expr.arguments.begin(), named_it);
      params[param_spec.name] = evaluate(*named_it->value);
      assigned[idx] = true;
      continue;
    }

    auto pos_it = std::find_if(expr.arguments.begin(), expr.arguments.end(),
                               [&](const atc::CallArg &arg) {
                                 size_t idx = &arg - expr.arguments.data();
                                 return !arg.name.has_value() && !assigned[idx];
                               });

    if (pos_it != expr.arguments.end()) {
      size_t idx = std::distance(expr.arguments.begin(), pos_it);
      params[param_spec.name] = evaluate(*pos_it->value);
      assigned[idx] = true;
      continue;
    }

    params[param_spec.name] = nullptr;
  }

  // Call function
  typing::FunctionResult result = (*func)(params);

  if (result.empty()) {
    return nullptr;
  }
  if (result.size() == 1) {
    return result[0];
  }
  return std::make_shared<typing::TupleValue>(result);
}

// ============================================================================
// OPERATORS
// ============================================================================

typing::RuntimeValue
ExprEvaluator::apply_binary_op(const std::string &op,
                               const typing::RuntimeValue &left,
                               const typing::RuntimeValue &right) {
  if (op == "+") {
    if (std::holds_alternative<int64_t>(left) &&
        std::holds_alternative<int64_t>(right)) {
      return std::get<int64_t>(left) + std::get<int64_t>(right);
    }
    if (std::holds_alternative<double>(left) ||
        std::holds_alternative<double>(right)) {
      double l = std::holds_alternative<double>(left) ? std::get<double>(left)
                                                      : std::get<int64_t>(left);
      double r = std::holds_alternative<double>(right)
                     ? std::get<double>(right)
                     : std::get<int64_t>(right);
      return l + r;
    }
    if (std::holds_alternative<std::string>(left) &&
        std::holds_alternative<std::string>(right)) {
      return std::get<std::string>(left) + std::get<std::string>(right);
    }
  }

  if (op == "-") {
    if (std::holds_alternative<int64_t>(left) &&
        std::holds_alternative<int64_t>(right)) {
      return std::get<int64_t>(left) - std::get<int64_t>(right);
    }
    if (std::holds_alternative<double>(left) ||
        std::holds_alternative<double>(right)) {
      double l = std::holds_alternative<double>(left) ? std::get<double>(left)
                                                      : std::get<int64_t>(left);
      double r = std::holds_alternative<double>(right)
                     ? std::get<double>(right)
                     : std::get<int64_t>(right);
      return l - r;
    }
  }

  if (op == "*") {
    if (std::holds_alternative<int64_t>(left) &&
        std::holds_alternative<int64_t>(right)) {
      return std::get<int64_t>(left) * std::get<int64_t>(right);
    }
    if (std::holds_alternative<double>(left) ||
        std::holds_alternative<double>(right)) {
      double l = std::holds_alternative<double>(left) ? std::get<double>(left)
                                                      : std::get<int64_t>(left);
      double r = std::holds_alternative<double>(right)
                     ? std::get<double>(right)
                     : std::get<int64_t>(right);
      return l * r;
    }
  }

  if (op == "/") {
    if (std::holds_alternative<int64_t>(left) &&
        std::holds_alternative<int64_t>(right)) {
      auto r = std::get<int64_t>(right);
      if (r == 0)
        throw EvaluationError("Division by zero");
      return std::get<int64_t>(left) / r;
    }
    if (std::holds_alternative<double>(left) ||
        std::holds_alternative<double>(right)) {
      double l = std::holds_alternative<double>(left) ? std::get<double>(left)
                                                      : std::get<int64_t>(left);
      double r = std::holds_alternative<double>(right)
                     ? std::get<double>(right)
                     : std::get<int64_t>(right);
      if (r == 0.0)
        throw EvaluationError("Division by zero");
      return l / r;
    }
  }

  if (op == "==") {
    return left == right;
  }
  if (op == "!=") {
    return left != right;
  }
  if (op == "<") {
    if (std::holds_alternative<int64_t>(left) &&
        std::holds_alternative<int64_t>(right)) {
      return std::get<int64_t>(left) < std::get<int64_t>(right);
    }
    if (std::holds_alternative<double>(left) ||
        std::holds_alternative<double>(right)) {
      double l = std::holds_alternative<double>(left) ? std::get<double>(left)
                                                      : std::get<int64_t>(left);
      double r = std::holds_alternative<double>(right)
                     ? std::get<double>(right)
                     : std::get<int64_t>(right);
      return l < r;
    }
  }
  if (op == ">") {
    if (std::holds_alternative<int64_t>(left) &&
        std::holds_alternative<int64_t>(right)) {
      return std::get<int64_t>(left) > std::get<int64_t>(right);
    }
    if (std::holds_alternative<double>(left) ||
        std::holds_alternative<double>(right)) {
      double l = std::holds_alternative<double>(left) ? std::get<double>(left)
                                                      : std::get<int64_t>(left);
      double r = std::holds_alternative<double>(right)
                     ? std::get<double>(right)
                     : std::get<int64_t>(right);
      return l > r;
    }
  }
  if (op == "<=") {
    if (std::holds_alternative<int64_t>(left) &&
        std::holds_alternative<int64_t>(right)) {
      return std::get<int64_t>(left) <= std::get<int64_t>(right);
    }
    if (std::holds_alternative<double>(left) ||
        std::holds_alternative<double>(right)) {
      double l = std::holds_alternative<double>(left) ? std::get<double>(left)
                                                      : std::get<int64_t>(left);
      double r = std::holds_alternative<double>(right)
                     ? std::get<double>(right)
                     : std::get<int64_t>(right);
      return l <= r;
    }
  }
  if (op == ">=") {
    if (std::holds_alternative<int64_t>(left) &&
        std::holds_alternative<int64_t>(right)) {
      return std::get<int64_t>(left) >= std::get<int64_t>(right);
    }
    if (std::holds_alternative<double>(left) ||
        std::holds_alternative<double>(right)) {
      double l = std::holds_alternative<double>(left) ? std::get<double>(left)
                                                      : std::get<int64_t>(left);
      double r = std::holds_alternative<double>(right)
                     ? std::get<double>(right)
                     : std::get<int64_t>(right);
      return l >= r;
    }
  }

  if (op == "&&") {
    return std::get<bool>(left) && std::get<bool>(right);
  }
  if (op == "||") {
    return std::get<bool>(left) || std::get<bool>(right);
  }

  throw EvaluationError("Unknown or incompatible binary operator: " + op);
}

typing::RuntimeValue
ExprEvaluator::apply_unary_op(const std::string &op,
                              const typing::RuntimeValue &operand) {
  if (op == "!") {
    return !std::get<bool>(operand);
  }
  if (op == "-") {
    if (std::holds_alternative<int64_t>(operand)) {
      return -std::get<int64_t>(operand);
    }
    if (std::holds_alternative<double>(operand)) {
      return -std::get<double>(operand);
    }
  }

  throw EvaluationError("Unknown or incompatible unary operator: " + op);
}

} // namespace falcon::dsl
