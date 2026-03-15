#include "falcon-lsp/DiagnosticsProvider.hpp"
#include "falcon-atc/AST.hpp"
#include <set>

namespace falcon::lsp {

// ---------------------------------------------------------------------------
// Helper: convert 1-indexed AST position to 0-indexed LSP range
// ---------------------------------------------------------------------------
static ::lsp::Range make_range(int line, int col, int len = 1) {
  unsigned l = line > 0 ? static_cast<unsigned>(line - 1) : 0u;
  unsigned c = col > 0 ? static_cast<unsigned>(col - 1) : 0u;
  return ::lsp::Range{
      ::lsp::Position{l, c},
      ::lsp::Position{l, c + static_cast<unsigned>(std::max(len, 1))}};
}

static ::lsp::Diagnostic make_error(const std::string &msg,
                                    const ::lsp::Range &rng,
                                    const std::string &source = "falcon-lsp") {
  ::lsp::Diagnostic d;
  d.range = rng;
  d.message = msg;
  d.severity = ::lsp::DiagnosticSeverityEnum{::lsp::DiagnosticSeverity::Error};
  d.source = source;
  return d;
}

static ::lsp::Diagnostic
make_warning(const std::string &msg, const ::lsp::Range &rng,
             const std::string &source = "falcon-lsp") {
  ::lsp::Diagnostic d;
  d.range = rng;
  d.message = msg;
  d.severity =
      ::lsp::DiagnosticSeverityEnum{::lsp::DiagnosticSeverity::Warning};
  d.source = source;
  return d;
}

// ---------------------------------------------------------------------------
// Semantic walker
// ---------------------------------------------------------------------------
class SemanticDiagnosticWalker {
public:
  explicit SemanticDiagnosticWalker(const FalconDocument &doc)
      : doc_(doc),
        registry_(falcon::atc::BuiltinFunctionRegistry::create_default()) {
    // Collect all symbol names declared in this document (and imports).
    for (const auto &sym : doc_.symbols) {
      known_names_.insert(sym.name);
    }
    // Names coming from resolved imports.
    for (const auto &n : doc_.imported_names) {
      known_names_.insert(n);
    }
  }

  std::vector<::lsp::Diagnostic> run() {
    if (!doc_.program)
      return diags_;

    for (const auto &at : doc_.program->autotuners)
      walk_autotuner(at);
    for (const auto &rt : doc_.program->routines)
      walk_routine(rt);
    for (const auto &st : doc_.program->structs)
      walk_struct(st);
    return diags_;
  }

private:
  const FalconDocument &doc_;
  falcon::atc::BuiltinFunctionRegistry registry_;
  std::set<std::string> known_names_;
  std::vector<::lsp::Diagnostic> diags_;

  // ── Walkers ──────────────────────────────────────────────────────────────

  void walk_autotuner(const falcon::atc::AutotunerDecl &at) {
    for (const auto &stmt : at.autotuner_variables)
      walk_stmt(*stmt);
    for (const auto &state : at.states) {
      for (const auto &stmt : state.body)
        walk_stmt(*stmt);
    }
  }

  void walk_routine(const falcon::atc::RoutineDecl &rt) {
    for (const auto &stmt : rt.body)
      walk_stmt(*stmt);
  }

  void walk_struct(const falcon::atc::StructDecl &st) {
    for (const auto &rt : st.routines)
      walk_routine(rt);
  }

  void walk_stmt(const falcon::atc::Stmt &stmt) {
    if (auto *s = dynamic_cast<const falcon::atc::AssignStmt *>(&stmt)) {
      walk_expr(*s->value);
    } else if (auto *s =
                   dynamic_cast<const falcon::atc::VarDeclStmt *>(&stmt)) {
      if (s->initializer)
        walk_expr(**s->initializer);
    } else if (auto *s = dynamic_cast<const falcon::atc::ExprStmt *>(&stmt)) {
      walk_expr(*s->expression);
    } else if (auto *s = dynamic_cast<const falcon::atc::IfStmt *>(&stmt)) {
      walk_expr(*s->condition);
      for (const auto &c : s->then_body)
        walk_stmt(*c);
      for (const auto &c : s->else_body)
        walk_stmt(*c);
    } else if (auto *s =
                   dynamic_cast<const falcon::atc::TransitionStmt *>(&stmt)) {
      for (const auto &p : s->parameters)
        if (p)
          walk_expr(*p);
    } else if (auto *s =
                   dynamic_cast<const falcon::atc::StructFieldAssignStmt *>(
                       &stmt)) {
      walk_expr(*s->object);
      walk_expr(*s->value);
    }
    // TerminalStmt – nothing to walk
  }

  void walk_expr(const falcon::atc::Expr &expr) {
    if (auto *e = dynamic_cast<const falcon::atc::CallExpr *>(&expr)) {
      check_call(*e);
      for (const auto &arg : e->arguments)
        if (arg.value)
          walk_expr(*arg.value);
    } else if (auto *e =
                   dynamic_cast<const falcon::atc::MethodCallExpr *>(&expr)) {
      walk_expr(*e->object);
      for (const auto &arg : e->args)
        if (arg)
          walk_expr(*arg);
    } else if (auto *e = dynamic_cast<const falcon::atc::BinaryExpr *>(&expr)) {
      walk_expr(*e->left);
      walk_expr(*e->right);
    } else if (auto *e = dynamic_cast<const falcon::atc::UnaryExpr *>(&expr)) {
      walk_expr(*e->operand);
    } else if (auto *e = dynamic_cast<const falcon::atc::MemberExpr *>(&expr)) {
      walk_expr(*e->object);
    } else if (auto *e = dynamic_cast<const falcon::atc::IndexExpr *>(&expr)) {
      walk_expr(*e->object);
      walk_expr(*e->index);
    }
    // LiteralExpr, NilLiteralExpr, VarExpr – leaves, nothing to recurse
  }

  // ── Specific checks ──────────────────────────────────────────────────────

  void check_call(const falcon::atc::CallExpr &call) {
    const std::string &name = call.name;

    // 1. Check builtin arity
    const auto *sig = registry_.lookup(name);
    if (sig) {
      size_t given = call.arguments.size();
      size_t required = 0;
      for (const auto &p : sig->parameters) {
        if (p.required)
          ++required;
      }
      size_t maximum = sig->parameters.size();

      if (given < required || given > maximum) {
        int len = static_cast<int>(name.size());
        auto rng = make_range(call.line, call.column, len);
        diags_.push_back(make_error(
            "'" + name + "' expects " +
                (required == maximum ? std::to_string(required)
                                     : std::to_string(required) + ".." +
                                           std::to_string(maximum)) +
                " argument(s), got " + std::to_string(given),
            rng));
      }
      return; // builtin – no further check needed
    }

    // 2. Check user-defined autotuner/routine (from symbol table + imports)
    // Look for a matching autotuner or routine symbol with param count info
    if (doc_.program) {
      // Check local autotuners
      auto *at_decl = doc_.program->autotuner_index.count(name)
                          ? doc_.program->autotuner_index.at(name)
                          : nullptr;
      if (at_decl) {
        size_t expected = at_decl->input_params.size();
        size_t given = call.arguments.size();
        if (given != expected) {
          auto rng =
              make_range(call.line, call.column, static_cast<int>(name.size()));
          diags_.push_back(make_error(
              "Autotuner '" + name + "' expects " + std::to_string(expected) +
                  " argument(s), got " + std::to_string(given),
              rng));
        }
        return;
      }

      // Check local routines
      auto *rt_decl = doc_.program->routine_index.count(name)
                          ? doc_.program->routine_index.at(name)
                          : nullptr;
      if (rt_decl) {
        size_t expected = rt_decl->input_params.size();
        size_t given = call.arguments.size();
        if (given != expected) {
          auto rng =
              make_range(call.line, call.column, static_cast<int>(name.size()));
          diags_.push_back(make_error(
              "Routine '" + name + "' expects " + std::to_string(expected) +
                  " argument(s), got " + std::to_string(given),
              rng));
        }
        return;
      }
    }

    // 3. Unknown function – warn only if the name is not in imported_names
    //    (we can't check arity for imported symbols without loading their AST,
    //     but we can at least skip false positives for known imported names).
    if (doc_.imported_names.count(name) == 0) {
      // Suppress for qualified names (Module::func) where the module is
      // imported
      const auto sep = name.find("::");
      if (sep != std::string::npos) {
        // e.g. "Config::get_group_plunger_gates" – treat as builtin-like
        return;
      }
      // Suppress for names visible in the symbol table under any kind
      for (const auto &sym : doc_.symbols) {
        if (sym.name == name)
          return;
      }
      if (call.line > 0) {
        auto rng =
            make_range(call.line, call.column, static_cast<int>(name.size()));
        diags_.push_back(
            make_warning("Unknown function or autotuner '" + name +
                             "' (not declared in this file or its imports)",
                         rng));
      }
    }
  }
};

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------
std::vector<::lsp::Diagnostic>
DiagnosticsProvider::diagnostics(const FalconDocument &doc) {
  std::vector<::lsp::Diagnostic> result;

  // 1. Syntax / parse errors (unchanged)
  for (const auto &err : doc.parse_errors) {
    ::lsp::Diagnostic diag;
    unsigned start_line =
        err.first_line > 0 ? static_cast<unsigned>(err.first_line - 1) : 0u;
    unsigned start_col =
        err.first_column > 0 ? static_cast<unsigned>(err.first_column - 1) : 0u;
    unsigned end_line = err.last_line > 0
                            ? static_cast<unsigned>(err.last_line - 1)
                            : start_line;
    unsigned end_col = err.last_column > 0
                           ? static_cast<unsigned>(err.last_column)
                           : start_col + 1;

    diag.range = ::lsp::Range{::lsp::Position{start_line, start_col},
                              ::lsp::Position{end_line, end_col}};
    diag.message = err.message;
    diag.severity =
        ::lsp::DiagnosticSeverityEnum{::lsp::DiagnosticSeverity::Error};
    diag.source = std::string{"falcon-lsp"};
    result.push_back(std::move(diag));
  }

  // 2. Semantic diagnostics (argument count, unknown symbols) – only when
  //    parsing succeeded so we have a valid AST with expression locations.
  if (doc.parse_errors.empty()) {
    SemanticDiagnosticWalker walker(doc);
    auto semantic = walker.run();
    result.insert(result.end(), std::make_move_iterator(semantic.begin()),
                  std::make_move_iterator(semantic.end()));
  }

  return result;
}

} // namespace falcon::lsp
