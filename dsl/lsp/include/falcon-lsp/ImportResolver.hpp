#pragma once

#include "FalconDocument.hpp"
#include <lsp/types.h>
#include <set>
#include <string>
#include <vector>

namespace falcon::lsp {

/**
 * @brief Tracks which imported files have been loaded/cached and emits
 *        Warning diagnostics for any import or ffimport whose source file
 *        has not yet been resolved.
 */
class ImportResolver {
public:
    /// Mark a path as resolved (its symbols are loaded/cached).
    void resolve(const std::string &path);

    /// Return true if path has been resolved.
    [[nodiscard]] bool is_resolved(const std::string &path) const;

    /// Emit Warning diagnostics for every import/ffimport in doc that is
    /// not yet resolved.  The diagnostic range covers the quoted path string
    /// on the matching source line, or falls back to position (0,0).
    [[nodiscard]] std::vector<::lsp::Diagnostic>
    check_imports(const FalconDocument &doc) const;

private:
    std::set<std::string> resolved_paths_;
};

} // namespace falcon::lsp
