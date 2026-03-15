#pragma once

#include "FalconDocument.hpp"
#include <lsp/types.h>
#include <vector>

namespace falcon::lsp {

class DiagnosticsProvider {
public:
    std::vector<::lsp::Diagnostic> diagnostics(const FalconDocument& doc);
};

} // namespace falcon::lsp
