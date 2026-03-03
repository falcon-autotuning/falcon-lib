#pragma once

#include "falcon-atc/AST.hpp"
#include "falcon-atc/Compiler.hpp"
#include "falcon-atc/ParseError.hpp"
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace falcon::lsp {

struct Symbol {
    std::string name;
    std::string kind; // "input_param", "output_param", "var", "state", "autotuner", "routine",
                      // "struct", "struct_field", "struct_routine", "import", "ffimport"
    std::string type_str;
    int line = 0; // 1-indexed (AST)
    int col = 0;  // 1-indexed (AST)
    std::string autotuner_name;
    std::string state_name;
};

struct FalconDocument {
    std::string uri;
    std::string text;
    std::unique_ptr<falcon::atc::Program> program;
    std::vector<falcon::atc::ParseError> parse_errors;
    std::vector<Symbol> symbols;
    std::vector<std::string> import_paths;   // from prog->imports
    std::vector<std::string> ffimport_paths; // from prog->ff_imports (wrapper files)
    std::set<std::string> imported_names;    // autotuner/routine names declared as available
                                             // (filled when imports are resolved; empty = not yet resolved)
};

class FalconDocumentParser {
public:
    FalconDocument parse(const std::string& uri, const std::string& text);
};

} // namespace falcon::lsp
