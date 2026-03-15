#include "falcon-lsp/ImportResolver.hpp"
#include "falcon-lsp/TypeChecker.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>

namespace falcon::lsp {

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

static ::lsp::Diagnostic make_import_diag(const std::string &path,
                                          unsigned start_line,
                                          unsigned start_col,
                                          unsigned end_col) {
  ::lsp::Diagnostic diag;
  diag.range = ::lsp::Range{::lsp::Position{start_line, start_col},
                            ::lsp::Position{start_line, end_col}};
  diag.message = "Import not yet loaded: " + path;
  diag.severity =
      ::lsp::DiagnosticSeverityEnum{::lsp::DiagnosticSeverity::Warning};
  diag.source = std::string{"falcon-lsp"};
  return diag;
}

/// Resolve @p import_path relative to the directory of @p importing_uri.
/// Returns empty string if resolution fails.
static std::string resolve_path(const std::string &importing_uri,
                                const std::string &import_path) {
  namespace fs = std::filesystem;

  // Strip leading "file://" if present
  std::string base = importing_uri;
  if (base.rfind("file://", 0) == 0) {
    base = base.substr(7);
  }

  try {
    fs::path base_dir = fs::path(base).parent_path();
    fs::path candidate = base_dir / import_path;
    candidate = fs::weakly_canonical(candidate);
    if (fs::exists(candidate)) {
      return candidate.string();
    }
  } catch (...) {
  }
  return {};
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

void ImportResolver::resolve(const std::string &path) {
  resolved_paths_.insert(path);
}

bool ImportResolver::is_resolved(const std::string &path) const {
  return resolved_paths_.count(path) > 0;
}

std::vector<::lsp::Diagnostic>
ImportResolver::check_imports(const FalconDocument &doc) const {
  std::vector<::lsp::Diagnostic> result;

  auto emit_for_path = [&](const std::string &path) {
    if (is_resolved(path))
      return;

    std::istringstream stream(doc.text);
    std::string line_text;
    unsigned line_num = 0;
    bool found = false;

    while (std::getline(stream, line_text)) {
      const std::string needle = '"' + path + '"';
      const auto pos = line_text.find(needle);
      if (pos != std::string::npos) {
        result.push_back(
            make_import_diag(path, line_num, static_cast<unsigned>(pos),
                             static_cast<unsigned>(pos + needle.size())));
        found = true;
        break;
      }
      ++line_num;
    }

    if (!found) {
      result.push_back(make_import_diag(path, 0, 0, 1));
    }
  };

  for (const auto &path : doc.import_paths)
    emit_for_path(path);
  for (const auto &path : doc.ffimport_paths)
    emit_for_path(path);

  return result;
}

void ImportResolver::resolve_document(const std::string &importing_uri,
                                      FalconDocument &doc,
                                      FalconDocumentParser &parser) {
  std::set<std::string> visited;
  // Seed with the current document so we don't re-parse it
  std::string self = importing_uri;
  if (self.rfind("file://", 0) == 0)
    self = self.substr(7);
  visited.insert(self);

  resolve_document_impl(importing_uri, doc, parser, visited);
}

// ─────────────────────────────────────────────────────────────────────────────
// Recursive implementation
// ─────────────────────────────────────────────────────────────────────────────

void ImportResolver::resolve_document_impl(const std::string &importing_uri,
                                           FalconDocument &doc,
                                           FalconDocumentParser &parser,
                                           std::set<std::string> &visited) {
  // ── Handle .fal imports ────────────────────────────────────────────────
  for (const auto &import_path : doc.import_paths) {
    if (is_resolved(import_path))
      continue;

    std::string abs_path = resolve_path(importing_uri, import_path);
    if (abs_path.empty()) {
      // Cannot find the file — leave unresolved (warning remains)
      continue;
    }

    // Cycle guard
    if (visited.count(abs_path)) {
      resolve(import_path); // mark so warning disappears
      continue;
    }
    visited.insert(abs_path);

    // Read the imported file
    std::ifstream f(abs_path);
    if (!f.is_open())
      continue;
    std::string src((std::istreambuf_iterator<char>(f)),
                    std::istreambuf_iterator<char>());

    // Parse
    std::string child_uri = "file://" + abs_path;
    FalconDocument child = parser.parse(child_uri, src);

    // Recurse into child's imports
    resolve_document_impl(child_uri, child, parser, visited);

    // Merge symbols into parent document
    for (auto &sym : child.symbols) {
      // Only export top-level declarations
      if (sym.kind == "autotuner" || sym.kind == "routine" ||
          sym.kind == "struct" || sym.kind == "struct_field" ||
          sym.kind == "struct_routine") {
        sym.from_import = true;
        doc.symbols.push_back(sym);
        doc.imported_names.insert(sym.name);
      }
    }

    resolve(import_path);
  }

  // ── Handle ffimport (C++ wrappers) ─────────────────────────────────────
  // We don't parse these, just mark them resolved to suppress warnings.
  for (const auto &path : doc.ffimport_paths) {
    if (!is_resolved(path)) {
      // Check if the file exists relative to the importer
      std::string abs_path = resolve_path(importing_uri, path);
      if (!abs_path.empty()) {
        resolve(path);
      }
    }
  }
}

} // namespace falcon::lsp
