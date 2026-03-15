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
    if (text[i] == '\n')
      ++cur_line;
  }

  size_t pos = line_start + character;
  if (pos >= text.size())
    return {};

  auto is_ident = [](char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
  };
  if (!is_ident(text[pos]))
    return {};

  size_t start = pos;
  while (start > 0 && is_ident(text[start - 1]))
    --start;
  size_t end = pos;
  while (end < text.size() && is_ident(text[end]))
    ++end;

  return text.substr(start, end - start);
}

std::optional<::lsp::Hover> HoverProvider::hover(const FalconDocument &doc,
                                                 const ::lsp::Position &pos) {
  std::string word = word_at(doc.text, pos.line, pos.character);
  if (word.empty())
    return std::nullopt;

  // ── 1. Built-in registry ─────────────────────────────────────────────
  auto registry = falcon::atc::BuiltinFunctionRegistry::create_default();
  const auto *builtin = registry.lookup(word);
  if (builtin) {
    std::ostringstream ss;
    ss << "**builtin** `" << builtin->qualified_name << "`\n\n";
    if (!builtin->parameters.empty()) {
      ss << "**Parameters:**";
      for (const auto &p : builtin->parameters) {
        ss << "\n- `" << p.name << "`: `" << p.type.to_string() << "`";
        if (!p.required)
          ss << " *(optional)*";
      }
    }
    if (!builtin->return_params.empty()) {
      ss << "\n\n**Returns:**";
      for (const auto &r : builtin->return_params) {
        ss << "\n- `" << r.name << "`: `" << r.type.to_string() << "`";
      }
    }
    ::lsp::Hover h;
    h.contents = ::lsp::MarkupContent{
        ::lsp::MarkupKindEnum{::lsp::MarkupKind::Markdown}, ss.str()};
    return h;
  }

  // ── 2. Document symbol table ─────────────────────────────────────────
  for (const auto &sym : doc.symbols) {
    if (sym.name != word)
      continue;

    std::ostringstream ss;
    const std::string import_note =
        sym.from_import ? "\n\n*(from import)*" : "";

    if (sym.kind == "struct") {
      ss << "**struct** `" << sym.name << "`" << import_note;

    } else if (sym.kind == "struct_field") {
      ss << "**field** `" << sym.name << "`: `" << sym.type_str << "`";
      if (!sym.autotuner_name.empty())
        ss << "\n\n*struct*: **" << sym.autotuner_name << "**";
      ss << import_note;

    } else if (sym.kind == "struct_routine") {
      ss << "**routine** `" << sym.name << "`";
      if (!sym.autotuner_name.empty())
        ss << " *(struct **" << sym.autotuner_name << "**)*";
      if (!sym.param_types.empty()) {
        ss << "\n\n**Parameters:**";
        for (const auto &pt : sym.param_types)
          ss << "\n- `" << pt << "`";
      }
      if (!sym.return_types.empty()) {
        ss << "\n\n**Returns:**";
        for (const auto &rt : sym.return_types)
          ss << "\n- `" << rt << "`";
      }
      ss << import_note;

    } else if (sym.kind == "autotuner") {
      ss << "**autotuner** `" << sym.name << "`";
      if (!sym.param_types.empty()) {
        ss << "\n\n**Inputs:**";
        for (const auto &pt : sym.param_types)
          ss << "\n- `" << pt << "`";
      }
      if (!sym.return_types.empty()) {
        ss << "\n\n**Outputs:**";
        for (const auto &rt : sym.return_types)
          ss << "\n- `" << rt << "`";
      }
      ss << import_note;

    } else if (sym.kind == "routine") {
      ss << "**routine** `" << sym.name << "`";
      if (!sym.autotuner_name.empty() && sym.autotuner_name != sym.name)
        ss << " *(in **" << sym.autotuner_name << "**)*";
      if (!sym.param_types.empty()) {
        ss << "\n\n**Parameters:**";
        for (const auto &pt : sym.param_types)
          ss << "\n- `" << pt << "`";
      }
      if (!sym.return_types.empty()) {
        ss << "\n\n**Returns:**";
        for (const auto &rt : sym.return_types)
          ss << "\n- `" << rt << "`";
      }
      ss << import_note;

    } else if (sym.kind == "input_param") {
      ss << "**param** `" << sym.name << "`: `" << sym.type_str << "`";
      if (!sym.autotuner_name.empty())
        ss << "\n\n*autotuner/routine*: **" << sym.autotuner_name << "**";
      if (!sym.state_name.empty())
        ss << " | *state*: **" << sym.state_name << "**";
      ss << import_note;

    } else if (sym.kind == "output_param") {
      ss << "**output param** `" << sym.name << "`: `" << sym.type_str << "`";
      if (!sym.autotuner_name.empty())
        ss << "\n\n*autotuner/routine*: **" << sym.autotuner_name << "**";
      ss << import_note;

    } else if (sym.kind == "var") {
      ss << "**var** `" << sym.name << "`: `" << sym.type_str << "`";
      if (!sym.autotuner_name.empty())
        ss << "\n\n*autotuner*: **" << sym.autotuner_name << "**";
      if (!sym.state_name.empty())
        ss << " | *state*: **" << sym.state_name << "**";
      ss << import_note;

    } else if (sym.kind == "state") {
      ss << "**state** `" << sym.name << "`";
      if (!sym.autotuner_name.empty())
        ss << "\n\n*autotuner*: **" << sym.autotuner_name << "**";
      ss << import_note;

    } else if (sym.kind == "import" || sym.kind == "ffimport") {
      ss << "**" << sym.kind << "** `" << sym.name << "`";

    } else {
      ss << "`" << sym.name << "`: " << sym.type_str << import_note;
    }

    const std::string content = ss.str();
    if (content.empty())
      continue;

    ::lsp::Hover h;
    h.contents = ::lsp::MarkupContent{
        ::lsp::MarkupKindEnum{::lsp::MarkupKind::Markdown}, content};
    return h;
  }

  return std::nullopt;
}

} // namespace falcon::lsp
