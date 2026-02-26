#include "TypeChecker.hpp"
#include "falcon-atc/AST.hpp"

namespace falcon::lsp {

void TypeChecker::analyze(const falcon::atc::Program& program) {
    symbols.clear();

    for (const auto& at : program.autotuners) {
        analyze_autotuner(at);
    }

    for (const auto& rt : program.routines) {
        Symbol s;
        s.name = rt.name;
        s.kind = "routine";
        s.type_str = "routine";
        s.line = 0;
        s.col = 0;
        symbols.push_back(std::move(s));

        for (const auto& p : rt.input_params) {
            Symbol ps;
            ps.name = p->name;
            ps.kind = "input_param";
            ps.type_str = p->type.to_string();
            ps.autotuner_name = rt.name;
            symbols.push_back(std::move(ps));
        }
        for (const auto& p : rt.output_params) {
            Symbol ps;
            ps.name = p->name;
            ps.kind = "output_param";
            ps.type_str = p->type.to_string();
            ps.autotuner_name = rt.name;
            symbols.push_back(std::move(ps));
        }
    }
}

void TypeChecker::analyze_autotuner(const falcon::atc::AutotunerDecl& at) {
    {
        Symbol s;
        s.name = at.name;
        s.kind = "autotuner";
        s.type_str = "autotuner";
        s.line = 0;
        s.col = 0;
        s.autotuner_name = at.name;
        symbols.push_back(std::move(s));
    }

    for (const auto& p : at.input_params) {
        Symbol s;
        s.name = p->name;
        s.kind = "input_param";
        s.type_str = p->type.to_string();
        s.autotuner_name = at.name;
        symbols.push_back(std::move(s));
    }

    for (const auto& p : at.output_params) {
        Symbol s;
        s.name = p->name;
        s.kind = "output_param";
        s.type_str = p->type.to_string();
        s.autotuner_name = at.name;
        symbols.push_back(std::move(s));
    }

    for (const auto& stmt : at.autotuner_variables) {
        analyze_stmt(*stmt, at.name, "");
    }

    for (const auto& state : at.states) {
        analyze_state(state, at.name);
    }
}

void TypeChecker::analyze_state(const falcon::atc::StateDecl& state,
                                  const std::string& autotuner_name) {
    {
        Symbol s;
        s.name = state.name;
        s.kind = "state";
        s.type_str = "state";
        s.autotuner_name = autotuner_name;
        s.state_name = state.name;
        symbols.push_back(std::move(s));
    }

    for (const auto& p : state.input_parameters) {
        Symbol s;
        s.name = p->name;
        s.kind = "input_param";
        s.type_str = p->type.to_string();
        s.autotuner_name = autotuner_name;
        s.state_name = state.name;
        symbols.push_back(std::move(s));
    }

    for (const auto& stmt : state.body) {
        analyze_stmt(*stmt, autotuner_name, state.name);
    }
}

void TypeChecker::analyze_stmt(const falcon::atc::Stmt& stmt,
                                 const std::string& autotuner_name,
                                 const std::string& state_name) {
    const auto* vd = dynamic_cast<const falcon::atc::VarDeclStmt*>(&stmt);
    if (vd) {
        Symbol s;
        s.name = vd->name;
        s.kind = "var";
        s.type_str = vd->type.to_string();
        s.line = vd->line;
        s.col = vd->column;
        s.autotuner_name = autotuner_name;
        s.state_name = state_name;
        symbols.push_back(std::move(s));
        return;
    }

    // Recurse into if-statement branches
    const auto* ifs = dynamic_cast<const falcon::atc::IfStmt*>(&stmt);
    if (ifs) {
        for (const auto& s : ifs->then_body) {
            analyze_stmt(*s, autotuner_name, state_name);
        }
        for (const auto& s : ifs->else_body) {
            analyze_stmt(*s, autotuner_name, state_name);
        }
    }
}

} // namespace falcon::lsp
