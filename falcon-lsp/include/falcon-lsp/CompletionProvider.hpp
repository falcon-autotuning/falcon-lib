#pragma once

#include "FalconDocument.hpp"
#include <lsp/types.h>
#include <vector>

namespace falcon::lsp {

class CompletionProvider {
public:
    std::vector<::lsp::CompletionItem> complete(const FalconDocument& doc,
                                                 const ::lsp::Position& pos);
};

} // namespace falcon::lsp
