#include "falcon-lsp/ImportResolver.hpp"
#include <sstream>

namespace falcon::lsp {

void ImportResolver::resolve(const std::string &path) {
    resolved_paths_.insert(path);
}

bool ImportResolver::is_resolved(const std::string &path) const {
    return resolved_paths_.count(path) > 0;
}

static ::lsp::Diagnostic make_import_diag(const std::string &path,
                                           unsigned start_line,
                                           unsigned start_col,
                                           unsigned end_col) {
    ::lsp::Diagnostic diag;
    diag.range = ::lsp::Range{
        ::lsp::Position{start_line, start_col},
        ::lsp::Position{start_line, end_col}};
    diag.message = "Import not yet loaded: " + path;
    diag.severity =
        ::lsp::DiagnosticSeverityEnum{::lsp::DiagnosticSeverity::Warning};
    diag.source = std::string{"falcon-lsp"};
    return diag;
}

std::vector<::lsp::Diagnostic>
ImportResolver::check_imports(const FalconDocument &doc) const {
    std::vector<::lsp::Diagnostic> result;

    auto emit_for_path = [&](const std::string &path) {
        if (is_resolved(path)) {
            return;
        }

        // Scan the document text line-by-line for the quoted path string.
        std::istringstream stream(doc.text);
        std::string line_text;
        unsigned line_num = 0;
        bool found = false;

        while (std::getline(stream, line_text)) {
            const std::string needle = '"' + path + '"';
            const auto pos = line_text.find(needle);
            if (pos != std::string::npos) {
                result.push_back(make_import_diag(
                    path, line_num, static_cast<unsigned>(pos),
                    static_cast<unsigned>(pos + needle.size())));
                found = true;
                break;
            }
            ++line_num;
        }

        if (!found) {
            // Fallback: emit at (0,0) when the path cannot be located in the text.
            result.push_back(make_import_diag(path, 0, 0, 1));
        }
    };

    for (const auto &path : doc.import_paths) {
        emit_for_path(path);
    }
    for (const auto &path : doc.ffimport_paths) {
        emit_for_path(path);
    }

    return result;
}

} // namespace falcon::lsp
