#pragma once

#include "FalconDocument.hpp"
#include <lsp/types.h>
#include <optional>

namespace falcon::lsp {

class DefinitionProvider {
public:
    std::optional<::lsp::Location> definition(const FalconDocument& doc,
                                               const ::lsp::Position& pos);

private:
    static std::string word_at(const std::string& text, unsigned line,
                                unsigned character);
};

} // namespace falcon::lsp
