#include "DefinitionProvider.hpp"

namespace falcon::lsp {

std::string DefinitionProvider::word_at(const std::string& text,
                                          unsigned line,
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

std::optional<::lsp::Location> DefinitionProvider::definition(
    const FalconDocument& doc,
    const ::lsp::Position& pos) {

    std::string word = word_at(doc.text, pos.line, pos.character);
    if (word.empty()) {
        return std::nullopt;
    }

    for (const auto& sym : doc.symbols) {
        if (sym.name == word && sym.line > 0) {
            unsigned lsp_line = static_cast<unsigned>(sym.line - 1);
            unsigned lsp_col = sym.col > 0
                                   ? static_cast<unsigned>(sym.col - 1)
                                   : 0u;
            ::lsp::Location loc;
            loc.uri = ::lsp::FileUri::fromPath(
                ::lsp::Uri::parse(doc.uri).path());
            loc.range = ::lsp::Range{
                ::lsp::Position{lsp_line, lsp_col},
                ::lsp::Position{lsp_line,
                                lsp_col + static_cast<unsigned>(word.size())}
            };
            return loc;
        }
    }

    return std::nullopt;
}

} // namespace falcon::lsp
