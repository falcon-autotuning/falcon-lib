#include "FalconDocument.hpp"
#include "TypeChecker.hpp"
#include "falcon-atc/ParseError.hpp"
#include <cstdio>
#include <filesystem>
#include <unistd.h>
#include <vector>

namespace falcon::lsp {

FalconDocument FalconDocumentParser::parse(const std::string& uri,
                                            const std::string& text) {
    FalconDocument doc;
    doc.uri = uri;
    doc.text = text;

    // Write to a temp file so parse_file() can read it.
    auto tmp_dir = std::filesystem::temp_directory_path();
    std::string tmp_template = (tmp_dir / "falcon-lsp-XXXXXX.fal").string();
    std::vector<char> tmp_buf(tmp_template.begin(), tmp_template.end());
    tmp_buf.push_back('\0');
    int fd = mkstemps(tmp_buf.data(), 4); // suffix ".fal"
    if (fd < 0) {
        falcon::atc::ParseError err;
        err.first_line = 1;
        err.first_column = 1;
        err.last_line = 1;
        err.last_column = 1;
        err.message = "Failed to create temp file for parsing";
        doc.parse_errors.push_back(std::move(err));
        return doc;
    }
    const std::string tmp_path(tmp_buf.data());
    {
        FILE* f = fdopen(fd, "w");
        if (f) {
            fwrite(text.data(), 1, text.size(), f);
            fclose(f);
        } else {
            close(fd);
        }
    }

    falcon::atc::current_errors.clear();
    falcon::atc::Compiler compiler;
    try {
        doc.program = compiler.parse_file(tmp_path);
        // Collect any non-fatal errors accumulated during parse
        doc.parse_errors = falcon::atc::current_errors;
    } catch (const std::exception&) {
        doc.parse_errors = falcon::atc::current_errors;
        if (doc.parse_errors.empty()) {
            doc.parse_errors = compiler.get_errors();
        }
        if (doc.parse_errors.empty()) {
            falcon::atc::ParseError err;
            err.first_line = 1;
            err.first_column = 1;
            err.last_line = 1;
            err.last_column = 1;
            err.message = "Parse failed";
            doc.parse_errors.push_back(std::move(err));
        }
    }

    std::remove(tmp_path.c_str());

    // Build symbol table from AST
    if (doc.program) {
        TypeChecker checker;
        checker.analyze(*doc.program);
        doc.symbols = std::move(checker.symbols);
    }

    return doc;
}

} // namespace falcon::lsp
