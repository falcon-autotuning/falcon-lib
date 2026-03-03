#include "falcon-lsp/HoverProvider.hpp"
#include "falcon-atc/AST.hpp"
#include <sstream>

namespace falcon::lsp {

// Extract the identifier word at (line, character) — both 0-indexed.
std::string HoverProvider::word_at(const std::string &text, unsigned line,
                                   unsigned character) {
  unsigned cur_line = 0;
  size_t line_start = 0;
  for (size_t i = 0; i < text.size(); ++i) {
    if (cur_line == line) {
      line_start = i;
      break;
    }
    if (text[i] == '\n') {
      ++cur_line;
    }
  }

  size_t pos = line_start + character;
  if (pos >= text.size()) {
    return {};
  }

  auto is_ident = [](char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
  };

  if (!is_ident(text[pos])) {
    return {};
  }

  size_t start = pos;
  while (start > 0 && is_ident(text[start - 1])) {
    --start;
  }
  size_t end = pos;
  while (end < text.size() && is_ident(text[end])) {
    ++end;
  }

  return text.substr(start, end - start);
}

std::optional<::lsp::Hover> HoverProvider::hover(const FalconDocument &doc,
                                                 const ::lsp::Position &pos) {
  std::string word = word_at(doc.text, pos.line, pos.character);
  if (word.empty()) {
    return std::nullopt;
  }

  // Look up in built-in registry first
  auto registry = falcon::atc::BuiltinFunctionRegistry::create_default();
  const auto *builtin = registry.lookup(word);
  if (builtin) {
    std::ostringstream ss;
    ss << "**builtin** `" << builtin->qualified_name << "`\n\n";
    ss << "Parameters:";
    for (const auto &p : builtin->parameters) {
      ss << "\n- `" << p.name << "`: " << p.type.to_string();
      if (!p.required)
        ss << " (optional)";
    }
    if (!builtin->return_params.empty()) {
      ss << "\n\nReturns:";
      for (const auto &r : builtin->return_params) {
        ss << "\n- `" << r.name << "`: " << r.type.to_string();
      }
    }
    ::lsp::Hover hover;
    hover.contents = ::lsp::MarkupContent{
        ::lsp::MarkupKindEnum{::lsp::MarkupKind::Markdown}, ss.str()};
    return hover;
  }

  // Look up in document symbol table
  for (const auto &sym : doc.symbols) {
    if (sym.name == word) {
      std::ostringstream ss;
      ss << "**" << sym.kind << "** `" << sym.name << "`";
      if (!sym.type_str.empty() && sym.kind != "autotuner" &&
          sym.kind != "state" && sym.kind != "routine") {
        ss << ": `" << sym.type_str << "`";
      }
      if (!sym.autotuner_name.empty()) {
        ss << "\n\n*autotuner*: " << sym.autotuner_name;
      }
      if (!sym.state_name.empty()) {
        ss << "\n\n*state*: " << sym.state_name;
      }
      ::lsp::Hover hover;
      hover.contents = ::lsp::MarkupContent{
          ::lsp::MarkupKindEnum{::lsp::MarkupKind::Markdown}, ss.str()};
      return hover;
    }
  }

  return std::nullopt;
}

} // namespace falcon::lsp
