#include "falcon-lsp/TypeChecker.hpp"
#include "falcon-atc/AST.hpp"

namespace falcon::lsp {

// ─────────────────────────────────────────────────────────────────────────────
// Forward declarations of helpers defined later in this file
// ─────────────────────────────────────────────────────────────────────────────
static void analyze_stmt(const falcon::atc::Stmt &stmt,
                         std::vector<Symbol> &symbols,
                         const std::string &autotuner_name,
                         const std::string &state_name);

static void analyze_state(const falcon::atc::StateDecl &state,
                          std::vector<Symbol> &symbols,
                          const std::string &autotuner_name);

static void analyze_routine_body(const falcon::atc::RoutineDecl &rt,
                                 std::vector<Symbol> &symbols,
                                 const std::string &owner_name);

// ─────────────────────────────────────────────────────────────────────────────
// Main entry point
// ─────────────────────────────────────────────────────────────────────────────

void TypeChecker::analyze(const falcon::atc::Program &program) {
  symbols.clear();
  import_paths.clear();
  ffimport_paths.clear();

  // ── Structs ─────────────────────────────────────────────────────────────
  for (const auto &st : program.structs) {
    {
      Symbol s;
      s.name = st.name;
      s.kind = "struct";
      s.type_str = "struct";
      symbols.push_back(std::move(s));
    }
    for (const auto &field : st.fields) {
      Symbol s;
      s.name = field.name;
      s.kind = "struct_field";
      s.type_str = field.type.to_string();
      s.line = field.line;
      s.col = field.column;
      s.autotuner_name = st.name;
      symbols.push_back(std::move(s));
    }
    for (const auto &rt : st.routines) {
      Symbol s;
      s.name = rt.name;
      s.kind = "struct_routine";
      s.type_str = "routine";
      s.autotuner_name = st.name;
      s.required_param_count = 0;
      for (const auto &p : rt.input_params) {
        s.param_types.push_back(p->type.to_string() + " " + p->name);
        ++s.required_param_count;
      }
      for (const auto &p : rt.output_params) {
        s.return_types.push_back(p->type.to_string() + " " + p->name);
      }
      symbols.push_back(std::move(s));
      analyze_routine_body(rt, symbols, st.name);
    }
  }

  // ── Autotuners ──────────────────────────────────────────────────────────
  for (const auto &at : program.autotuners) {
    analyze_autotuner(at);
  }

  // ── Top-level routines ──────────────────────────────────────────────────
  for (const auto &rt : program.routines) {
    Symbol s;
    s.name = rt.name;
    s.kind = "routine";
    s.type_str = "routine";
    s.required_param_count = 0;
    for (const auto &p : rt.input_params) {
      s.param_types.push_back(p->type.to_string() + " " + p->name);
      ++s.required_param_count;
    }
    for (const auto &p : rt.output_params) {
      s.return_types.push_back(p->type.to_string() + " " + p->name);
    }
    symbols.push_back(std::move(s));
    analyze_routine_body(rt, symbols, rt.name);
  }

  // ── Imports ─────────────────────────────────────────────────────────────
  for (const auto &path : program.imports) {
    Symbol s;
    s.name = path;
    s.kind = "import";
    s.type_str = "import";
    symbols.push_back(std::move(s));
    import_paths.push_back(path);
  }

  for (const auto &ffi : program.ff_imports) {
    Symbol s;
    s.name = ffi.wrapper_file;
    s.kind = "ffimport";
    s.type_str = "ffimport";
    symbols.push_back(std::move(s));
    ffimport_paths.push_back(ffi.wrapper_file);
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Autotuner
// ─────────────────────────────────────────────────────────────────────────────

void TypeChecker::analyze_autotuner(const falcon::atc::AutotunerDecl &at) {
  {
    Symbol s;
    s.name = at.name;
    s.kind = "autotuner";
    s.type_str = "autotuner";
    s.autotuner_name = at.name;
    s.required_param_count = 0;
    for (const auto &p : at.input_params) {
      s.param_types.push_back(p->type.to_string() + " " + p->name);
      ++s.required_param_count;
    }
    for (const auto &p : at.output_params) {
      s.return_types.push_back(p->type.to_string() + " " + p->name);
    }
    symbols.push_back(std::move(s));
  }

  for (const auto &p : at.input_params) {
    Symbol s;
    s.name = p->name;
    s.kind = "input_param";
    s.type_str = p->type.to_string();
    s.autotuner_name = at.name;
    symbols.push_back(std::move(s));
  }

  for (const auto &p : at.output_params) {
    Symbol s;
    s.name = p->name;
    s.kind = "output_param";
    s.type_str = p->type.to_string();
    s.autotuner_name = at.name;
    symbols.push_back(std::move(s));
  }

  for (const auto &stmt : at.autotuner_variables) {
    ::falcon::lsp::analyze_stmt(*stmt, symbols, at.name, "");
  }

  for (const auto &state : at.states) {
    ::falcon::lsp::analyze_state(state, symbols, at.name);
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Static helpers
// ─────────────────────────────────────────────────────────────────────────────

static void analyze_stmt(const falcon::atc::Stmt &stmt,
                         std::vector<Symbol> &symbols,
                         const std::string &autotuner_name,
                         const std::string &state_name) {
  if (const auto *decl =
          dynamic_cast<const falcon::atc::VarDeclStmt *>(&stmt)) {
    Symbol s;
    s.name = decl->name;
    s.kind = "var";
    s.type_str = decl->type.to_string();
    s.line = decl->line;
    s.col = decl->column;
    s.autotuner_name = autotuner_name;
    s.state_name = state_name;
    symbols.push_back(std::move(s));
  } else if (const auto *ifst =
                 dynamic_cast<const falcon::atc::IfStmt *>(&stmt)) {
    for (const auto &s : ifst->then_body)
      analyze_stmt(*s, symbols, autotuner_name, state_name);
    for (const auto &s : ifst->else_body)
      analyze_stmt(*s, symbols, autotuner_name, state_name);
  }
}

static void analyze_state(const falcon::atc::StateDecl &state,
                          std::vector<Symbol> &symbols,
                          const std::string &autotuner_name) {
  {
    Symbol s;
    s.name = state.name;
    s.kind = "state";
    s.type_str = "state";
    s.autotuner_name = autotuner_name;
    symbols.push_back(std::move(s));
  }

  for (const auto &p : state.input_parameters) {
    Symbol s;
    s.name = p->name;
    s.kind = "input_param";
    s.type_str = p->type.to_string();
    s.autotuner_name = autotuner_name;
    s.state_name = state.name;
    symbols.push_back(std::move(s));
  }

  for (const auto &stmt : state.body) {
    analyze_stmt(*stmt, symbols, autotuner_name, state.name);
  }
}

static void analyze_routine_body(const falcon::atc::RoutineDecl &rt,
                                 std::vector<Symbol> &symbols,
                                 const std::string &owner_name) {
  for (const auto &p : rt.input_params) {
    Symbol s;
    s.name = p->name;
    s.kind = "input_param";
    s.type_str = p->type.to_string();
    s.autotuner_name = owner_name;
    symbols.push_back(std::move(s));
  }
  for (const auto &p : rt.output_params) {
    Symbol s;
    s.name = p->name;
    s.kind = "output_param";
    s.type_str = p->type.to_string();
    s.autotuner_name = owner_name;
    symbols.push_back(std::move(s));
  }
  for (const auto &stmt : rt.body) {
    analyze_stmt(*stmt, symbols, owner_name, "");
  }
}

} // namespace falcon::lsp
