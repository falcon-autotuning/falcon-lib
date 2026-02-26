#pragma once

#include "FalconDocument.hpp"
#include <lsp/connection.h>
#include <lsp/messagehandler.h>
#include <map>
#include <string>

namespace falcon::lsp {

class FalconServer {
public:
    explicit FalconServer(::lsp::MessageHandler& handler);

private:
    ::lsp::MessageHandler& handler_;
    std::map<std::string, FalconDocument> docs_;
    FalconDocumentParser parser_;

    void setup_handlers();
    FalconDocument& analyze(const std::string& uri, const std::string& text);
    void publish_diagnostics(const std::string& uri,
                              const FalconDocument& doc);
};

} // namespace falcon::lsp
