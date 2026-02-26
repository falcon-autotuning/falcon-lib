#pragma once

#include "FalconDocument.hpp"
#include <lsp/types.h>
#include <optional>
#include <string>

namespace falcon::lsp {

class HoverProvider {
public:
    std::optional<::lsp::Hover> hover(const FalconDocument& doc,
                                       const ::lsp::Position& pos);

private:
    static std::string word_at(const std::string& text, unsigned line,
                                unsigned character);
};

} // namespace falcon::lsp
