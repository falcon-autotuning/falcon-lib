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
 *
 *        Also provides recursive import resolution: given a document with
 *        import paths, it parses each imported file, collects their top-level
 *        symbols, and merges them into the parent document.
 */
class ImportResolver {
public:
  /// Mark a path as resolved (its symbols are loaded / cached).
  void resolve(const std::string &path);

  /// Return true if path has been resolved.
  [[nodiscard]] bool is_resolved(const std::string &path) const;

  /// Emit Warning diagnostics for every import/ffimport in doc that is
  /// not yet resolved.  The diagnostic range covers the quoted path string
  /// on the matching source line, or falls back to position (0,0).
  [[nodiscard]] std::vector<::lsp::Diagnostic>
  check_imports(const FalconDocument &doc) const;

  /**
   * @brief Recursively resolve all imports of @p doc.
   *
   * For each path in doc.import_paths:
   *   1. Resolve the path relative to the directory of @p importing_uri.
   *   2. Parse the file using @p parser.
   *   3. Recursively call resolve_document for the parsed child (cycle guard).
   *   4. Merge autotuner/routine/struct symbols into @p doc.symbols,
   *      marking them with from_import = true.
   *   5. Call resolve(path) so check_imports() no longer warns.
   *
   * ffimport paths are marked resolved but not parsed (they are C++ wrappers).
   *
   * @param importing_uri  URI/path of the document that contains the imports.
   * @param doc            Document whose imports should be resolved in-place.
   * @param parser         Parser to use for imported files.
   */
  void resolve_document(const std::string &importing_uri, FalconDocument &doc,
                        FalconDocumentParser &parser);

private:
  std::set<std::string> resolved_paths_;

  /// Recursive helper; @p visited prevents infinite loops.
  void resolve_document_impl(const std::string &importing_uri,
                             FalconDocument &doc, FalconDocumentParser &parser,
                             std::set<std::string> &visited);
};

} // namespace falcon::lsp
