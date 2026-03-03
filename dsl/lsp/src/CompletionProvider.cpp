#include "falcon-lsp/CompletionProvider.hpp"
#include "falcon-atc/AST.hpp"

namespace falcon::lsp {

static const std::vector<std::string> KEYWORDS = {"autotuner",
                                                  "routine",
                                                  "state",
                                                  "start",
                                                  "terminal",
                                                  "if",
                                                  "else",
                                                  "true",
                                                  "false",
                                                  "nil",
                                                  "int",
                                                  "float",
                                                  "bool",
                                                  "string",
                                                  "Quantity",
                                                  "Config",
                                                  "Connection",
                                                  "Connections",
                                                  "Gname",
                                                  "DeviceCharacteristic",
                                                  "DeviceCharacteristicQuery",
                                                  "Error",
                                                  "FatalError"};

std::vector<::lsp::CompletionItem>
CompletionProvider::complete(const FalconDocument &doc,
                             const ::lsp::Position & /*pos*/) {

  std::vector<::lsp::CompletionItem> items;

  // Add keywords
  for (const auto &kw : KEYWORDS) {
    ::lsp::CompletionItem item;
    item.label = kw;
    item.kind =
        ::lsp::CompletionItemKindEnum{::lsp::CompletionItemKind::Keyword};
    items.push_back(std::move(item));
  }

  // Add built-in functions
  auto registry = falcon::atc::BuiltinFunctionRegistry::create_default();
  // We enumerate known builtins from the registry by querying common names
  static const std::vector<std::string> BUILTIN_NAMES = {
      "logInfo",
      "logWarn",
      "logError",
      "errorMsg",
      "fatalErrorMsg",
      "readLatest",
      "write",
      "Config::get_group_plunger_gates",
      "Config::get_group_barrier_gates",
      "Config::get_group_reservoir_gates"};
  for (const auto &name : BUILTIN_NAMES) {
    const auto *sig = registry.lookup(name);
    if (sig) {
      ::lsp::CompletionItem item;
      item.label = name;
      item.kind =
          ::lsp::CompletionItemKindEnum{::lsp::CompletionItemKind::Function};
      item.detail = "builtin";
      items.push_back(std::move(item));
    }
  }

  // Add document symbols
  for (const auto &sym : doc.symbols) {
    ::lsp::CompletionItem item;
    item.label = sym.name;
    if (sym.kind == "autotuner") {
      item.kind =
          ::lsp::CompletionItemKindEnum{::lsp::CompletionItemKind::Class};
    } else if (sym.kind == "state") {
      item.kind =
          ::lsp::CompletionItemKindEnum{::lsp::CompletionItemKind::Module};
    } else if (sym.kind == "routine") {
      item.kind =
          ::lsp::CompletionItemKindEnum{::lsp::CompletionItemKind::Function};
    } else if (sym.kind == "input_param" || sym.kind == "output_param" ||
               sym.kind == "var") {
      item.kind =
          ::lsp::CompletionItemKindEnum{::lsp::CompletionItemKind::Variable};
      item.detail = sym.type_str;
    }
    items.push_back(std::move(item));
  }

  return items;
}

} // namespace falcon::lsp
