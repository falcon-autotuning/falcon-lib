#include "falcon-lsp/SemanticAnalyzer.hpp"
#include "falcon-atc/AST.hpp"
#include <map>
#include <set>
#include <string>

namespace falcon::lsp {

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

static ::lsp::Diagnostic make_semantic_diag(const std::string &message,
                                            int line, int col) {
  unsigned lsp_line = line > 0 ? static_cast<unsigned>(line - 1) : 0u;
  unsigned lsp_col = col > 0 ? static_cast<unsigned>(col - 1) : 0u;

  ::lsp::Diagnostic diag;
  diag.range = ::lsp::Range{
      .start = ::lsp::Position{.line = lsp_line, .character = lsp_col},
      .end = ::lsp::Position{.line = lsp_line, .character = lsp_col + 1}};
  diag.message = message;
  diag.severity =
      ::lsp::DiagnosticSeverityEnum{::lsp::DiagnosticSeverity::Error};
  diag.source = std::string{"falcon-lsp"};
  return diag;
}

// ─────────────────────────────────────────────────────────────────────────────
// Call-site walker
// ─────────────────────────────────────────────────────────────────────────────

struct CallChecker {
  /// known_callables: name → required param count (-1 = variadic / unknown)
  std::map<std::string, int> known_callables;
  std::vector<::lsp::Diagnostic> &diags;

  void check_call(const std::string &name, int arg_count, int stmt_line,
                  int stmt_col) {
    auto it = known_callables.find(name);
    if (it == known_callables.end()) {
      diags.push_back(
          make_semantic_diag("Undefined: '" + name + "'", stmt_line, stmt_col));
      return;
    }
    int expected = it->second;
    if (expected >= 0 && arg_count != expected) {
      diags.push_back(make_semantic_diag(
          "'" + name + "' expects " + std::to_string(expected) +
              " argument(s), got " + std::to_string(arg_count),
          stmt_line, stmt_col));
    }
  }

  void walk_expr(const falcon::atc::Expr &expr) {
    if (const auto *call = dynamic_cast<const falcon::atc::CallExpr *>(&expr)) {
      int argc = static_cast<int>(call->arguments.size());
      int line = call->line, col = call->column;
      check_call(call->name, argc, line, col);
      for (const auto &arg : call->arguments) {
        if (arg.value)
          walk_expr(*arg.value);
      }
    } else if (const auto *bin =
                   dynamic_cast<const falcon::atc::BinaryExpr *>(&expr)) {
      walk_expr(*bin->left);
      walk_expr(*bin->right);
    } else if (const auto *un =
                   dynamic_cast<const falcon::atc::UnaryExpr *>(&expr)) {
      walk_expr(*un->operand);
    } else if (const auto *mem =
                   dynamic_cast<const falcon::atc::MethodCallExpr *>(&expr)) {
      walk_expr(*mem->object);
      for (const auto &a : mem->args)
        walk_expr(*a);
    } else if (const auto *idx =
                   dynamic_cast<const falcon::atc::IndexExpr *>(&expr)) {
      walk_expr(*idx->object);
      walk_expr(*idx->index);
    } else if (const auto *ma =
                   dynamic_cast<const falcon::atc::MemberExpr *>(&expr)) {
      walk_expr(*ma->object);
    }
  }

  void walk_stmt(const falcon::atc::Stmt &stmt) {
    if (const auto *es = dynamic_cast<const falcon::atc::ExprStmt *>(&stmt)) {
      walk_expr(*es->expression);
    } else if (const auto *as =
                   dynamic_cast<const falcon::atc::AssignStmt *>(&stmt)) {
      walk_expr(*as->value);
    } else if (const auto *vd =
                   dynamic_cast<const falcon::atc::VarDeclStmt *>(&stmt)) {
      if (vd->initializer)
        walk_expr(**vd->initializer);
    } else if (const auto *ifs =
                   dynamic_cast<const falcon::atc::IfStmt *>(&stmt)) {
      walk_expr(*ifs->condition);
      for (const auto &s : ifs->then_body)
        walk_stmt(*s);
      for (const auto &s : ifs->else_body)
        walk_stmt(*s);
    } else if (const auto *tr =
                   dynamic_cast<const falcon::atc::TransitionStmt *>(&stmt)) {
      for (const auto &p : tr->parameters)
        walk_expr(*p);
    } else if (const auto *sfa =
                   dynamic_cast<const falcon::atc::StructFieldAssignStmt *>(
                       &stmt)) {
      walk_expr(*sfa->object);
      walk_expr(*sfa->value);
    }
  }

  void
  walk_stmts(const std::vector<std::unique_ptr<falcon::atc::Stmt>> &stmts) {
    for (const auto &s : stmts)
      walk_stmt(*s);
  }
};

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

std::vector<::lsp::Diagnostic>
SemanticAnalyzer::analyze(const FalconDocument &doc) {
  std::vector<::lsp::Diagnostic> result;

  if (!doc.program)
    return result;

  // ── Build callable map ───────────────────────────────────────────────
  std::map<std::string, int> known; // name → required param count

  // Built-ins
  auto registry = falcon::atc::BuiltinFunctionRegistry::create_default();
  // We iterate all symbols and check builtins separately per call site,
  // but we also need to seed from the registry.  Since BuiltinFunctionRegistry
  // has no iterator, we seed from the document's completion list by querying
  // the registry for each symbol kind "builtin" — or we directly query at
  // call time.  Here we seed from a hard-coded known-names pass + registry
  // lookup at check_call time by leaving required_param_count = -1 for
  // builtins looked up from the registry.

  // Seed from doc symbols (autotuners, routines, struct_routines)
  for (const auto &sym : doc.symbols) {
    if (sym.kind == "autotuner" || sym.kind == "routine") {
      known[sym.name] = sym.required_param_count;
    }
    // struct_routines are called as qualified Struct::Routine, skip here
  }

  // For each CallExpr we'll check the registry at call time:
  // Build a second map that defers to the registry for un-found names.
  CallChecker checker{std::move(known), result};

  // Override check_call to also consult the builtin registry for unknown names
  auto check_call_extended = [&](const std::string &name, int arg_count,
                                 int stmt_line, int stmt_col) {
    auto it = checker.known_callables.find(name);
    if (it == checker.known_callables.end()) {
      // Check builtin registry
      const auto *sig = registry.lookup(name);
      if (sig) {
        // Known builtin — check arg count
        int req = 0;
        bool variadic = false;
        for (const auto &p : sig->parameters) {
          if (p.required)
            ++req;
          else {
            variadic = true;
            break;
          }
        }
        if (!variadic && arg_count < req) {
          result.push_back(make_semantic_diag(
              "'" + name + "' expects at least " + std::to_string(req) +
                  " argument(s), got " + std::to_string(arg_count),
              stmt_line, stmt_col));
        }
        return;
      }
      // Truly undefined — but only flag explicit function-call expressions
      // (not method calls on objects, which use MethodCallExpr)
      result.push_back(
          make_semantic_diag("Undefined: '" + name + "'", stmt_line, stmt_col));
      return;
    }
    int expected = it->second;
    if (expected >= 0 && arg_count != expected) {
      result.push_back(make_semantic_diag(
          "'" + name + "' expects " + std::to_string(expected) +
              " argument(s), got " + std::to_string(arg_count),
          stmt_line, stmt_col));
    }
  };

  // Re-implement walk using the extended checker as a lambda
  std::function<void(const falcon::atc::Expr &)> walk_expr;
  std::function<void(const falcon::atc::Stmt &)> walk_stmt;

  walk_expr = [&](const falcon::atc::Expr &expr) {
    if (const auto *call = dynamic_cast<const falcon::atc::CallExpr *>(&expr)) {
      int argc = static_cast<int>(call->arguments.size());
      check_call_extended(call->name, argc, call->line, call->column);
      for (const auto &arg : call->arguments)
        if (arg.value)
          walk_expr(*arg.value);
    } else if (const auto *bin =
                   dynamic_cast<const falcon::atc::BinaryExpr *>(&expr)) {
      walk_expr(*bin->left);
      walk_expr(*bin->right);
    } else if (const auto *un =
                   dynamic_cast<const falcon::atc::UnaryExpr *>(&expr)) {
      walk_expr(*un->operand);
    } else if (const auto *mc =
                   dynamic_cast<const falcon::atc::MethodCallExpr *>(&expr)) {
      walk_expr(*mc->object);
      for (const auto &a : mc->args)
        walk_expr(*a);
    } else if (const auto *idx =
                   dynamic_cast<const falcon::atc::IndexExpr *>(&expr)) {
      walk_expr(*idx->object);
      walk_expr(*idx->index);
    } else if (const auto *ma =
                   dynamic_cast<const falcon::atc::MemberExpr *>(&expr)) {
      walk_expr(*ma->object);
    }
  };

  walk_stmt = [&](const falcon::atc::Stmt &stmt) {
    if (const auto *es = dynamic_cast<const falcon::atc::ExprStmt *>(&stmt)) {
      walk_expr(*es->expression);
    } else if (const auto *as =
                   dynamic_cast<const falcon::atc::AssignStmt *>(&stmt)) {
      walk_expr(*as->value);
    } else if (const auto *vd =
                   dynamic_cast<const falcon::atc::VarDeclStmt *>(&stmt)) {
      if (vd->initializer)
        walk_expr(**vd->initializer);
    } else if (const auto *ifs =
                   dynamic_cast<const falcon::atc::IfStmt *>(&stmt)) {
      walk_expr(*ifs->condition);
      for (const auto &s : ifs->then_body)
        walk_stmt(*s);
      for (const auto &s : ifs->else_body)
        walk_stmt(*s);
    } else if (const auto *tr =
                   dynamic_cast<const falcon::atc::TransitionStmt *>(&stmt)) {
      for (const auto &p : tr->parameters)
        walk_expr(*p);
    } else if (const auto *sfa =
                   dynamic_cast<const falcon::atc::StructFieldAssignStmt *>(
                       &stmt)) {
      walk_expr(*sfa->object);
      walk_expr(*sfa->value);
    }
  };

  auto walk_stmts =
      [&](const std::vector<std::unique_ptr<falcon::atc::Stmt>> &stmts) {
        for (const auto &s : stmts)
          walk_stmt(*s);
      };

  // ── Walk every autotuner ──────────────────────────────────────────────
  for (const auto &at : doc.program->autotuners) {
    walk_stmts(at.autotuner_variables);
    for (const auto &state : at.states) {
      walk_stmts(state.body);
    }
  }

  // ── Walk every top-level routine ─────────────────────────────────────
  for (const auto &rt : doc.program->routines) {
    walk_stmts(rt.body);
  }

  // ── Walk struct routines ─────────────────────────────────────────────
  for (const auto &st : doc.program->structs) {
    for (const auto &rt : st.routines) {
      walk_stmts(rt.body);
    }
  }

  return result;
}

} // namespace falcon::lsp
