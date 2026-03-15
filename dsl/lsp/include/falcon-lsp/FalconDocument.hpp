#pragma once

#include "falcon-atc/AST.hpp"
#include "falcon-atc/ParseError.hpp"
#include <string>
#include <vector>

namespace falcon::lsp {

struct Symbol {
  std::string name;
  /// "input_param", "output_param", "var", "state", "autotuner", "routine",
  /// "struct", "struct_field", "struct_routine", "import", "ffimport"
  std::string kind;
  std::string type_str;
  int line = 0;               // 1-indexed (AST)
  int col = 0;                // 1-indexed (AST)
  std::string autotuner_name; // owning autotuner / struct name
  std::string state_name;

  // For autotuner / routine symbols: human-readable param type strings
  std::vector<std::string> param_types;  // input param types
  std::vector<std::string> return_types; // output param types
  // Number of *required* input parameters (used by SemanticAnalyzer)
  int required_param_count = -1; // -1 = unknown / variadic

  // True when this symbol was collected from an imported file, not the
  // currently-edited document.
  bool from_import = false;
};

struct FalconDocument {
  std::string uri;
  std::string text;
  std::unique_ptr<falcon::atc::Program> program;
  std::vector<falcon::atc::ParseError> parse_errors;
  std::vector<Symbol> symbols;
  std::vector<std::string> import_paths; // from prog->imports
  std::vector<std::string>
      ffimport_paths; // from prog->ff_imports (wrapper files)
  /// Autotuner/routine/struct names visible to this document (own + imported)
  std::set<std::string> imported_names;
};

class FalconDocumentParser {
public:
  FalconDocument parse(const std::string &uri, const std::string &text);
};

} // namespace falcon::lsp
