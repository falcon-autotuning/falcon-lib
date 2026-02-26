#include "falcon-lsp/FalconServer.hpp"
#include "falcon-lsp/CompletionProvider.hpp"
#include "falcon-lsp/DefinitionProvider.hpp"
#include "falcon-lsp/DiagnosticsProvider.hpp"
#include "falcon-lsp/HoverProvider.hpp"
#include <lsp/messages.h>
#include <lsp/types.h>

namespace falcon::lsp {

FalconServer::FalconServer(::lsp::MessageHandler &handler) : handler_(handler) {
  setup_handlers();
}

FalconDocument &FalconServer::analyze(const std::string &uri,
                                      const std::string &text) {
  docs_[uri] = parser_.parse(uri, text);
  return docs_[uri];
}

void FalconServer::publish_diagnostics(const std::string &uri,
                                       const FalconDocument &doc) {
  DiagnosticsProvider dp;
  ::lsp::PublishDiagnosticsParams params;
  params.uri = ::lsp::FileUri::fromPath(::lsp::Uri::parse(uri).path());
  params.diagnostics = dp.diagnostics(doc);
  handler_
      .sendNotification<::lsp::notifications::TextDocument_PublishDiagnostics>(
          std::move(params));
}

void FalconServer::setup_handlers() {
  handler_.add<::lsp::requests::Initialize>(
      [](::lsp::requests::Initialize::Params &&) {
        ::lsp::InitializeResult result;

        ::lsp::TextDocumentSyncOptions sync_opts;
        sync_opts.openClose = true;
        sync_opts.change =
            ::lsp::TextDocumentSyncKindEnum{::lsp::TextDocumentSyncKind::Full};

        result.capabilities.textDocumentSync = sync_opts;
        result.capabilities.hoverProvider = true;
        result.capabilities.completionProvider = ::lsp::CompletionOptions{};
        result.capabilities.definitionProvider = true;

        ::lsp::InitializeResultServerInfo info;
        info.name = "falcon-lsp";
        info.version = std::string{"0.1.0"};
        result.serverInfo = std::move(info);

        return result;
      });

  handler_.add<::lsp::notifications::TextDocument_DidOpen>(
      [this](::lsp::notifications::TextDocument_DidOpen::Params &&params) {
        const std::string uri = std::string{params.textDocument.uri.path()};
        const std::string &text = params.textDocument.text;
        // Use original URI string for storage but file path for parsing
        auto &doc = analyze(std::string{params.textDocument.uri.path()}, text);
        doc.uri = std::string{params.textDocument.uri.path()};
        publish_diagnostics(doc.uri, doc);
      });

  handler_.add<::lsp::notifications::TextDocument_DidChange>(
      [this](::lsp::notifications::TextDocument_DidChange::Params &&params) {
        if (params.contentChanges.empty()) {
          return;
        }
        std::string text;
        // Full sync: last change has the whole document
        const auto &change = params.contentChanges.back();
        if (std::holds_alternative<::lsp::TextDocumentContentChangeEvent_Text>(
                change)) {
          text =
              std::get<::lsp::TextDocumentContentChangeEvent_Text>(change).text;
        } else {
          text =
              std::get<::lsp::TextDocumentContentChangeEvent_Range_Text>(change)
                  .text;
        }
        const std::string uri = std::string{params.textDocument.uri.path()};
        auto &doc = analyze(uri, text);
        doc.uri = uri;
        publish_diagnostics(uri, doc);
      });

  handler_.add<::lsp::notifications::TextDocument_DidSave>(
      [this](::lsp::notifications::TextDocument_DidSave::Params &&params) {
        const std::string uri = std::string{params.textDocument.uri.path()};
        auto it = docs_.find(uri);
        if (it != docs_.end()) {
          publish_diagnostics(uri, it->second);
        }
      });

  handler_.add<::lsp::requests::TextDocument_Hover>(
      [this](::lsp::requests::TextDocument_Hover::Params &&params)
          -> ::lsp::requests::TextDocument_Hover::Result {
        const std::string uri = std::string{params.textDocument.uri.path()};
        auto it = docs_.find(uri);
        if (it == docs_.end()) {
          return nullptr;
        }
        HoverProvider hp;
        auto result = hp.hover(it->second, params.position);
        if (!result) {
          return nullptr;
        }
        return *result;
      });

  handler_.add<::lsp::requests::TextDocument_Completion>(
      [this](::lsp::requests::TextDocument_Completion::Params &&params)
          -> ::lsp::requests::TextDocument_Completion::Result {
        const std::string uri = std::string{params.textDocument.uri.path()};
        auto it = docs_.find(uri);
        if (it == docs_.end()) {
          return nullptr;
        }
        CompletionProvider cp;
        auto items = cp.complete(it->second, params.position);
        return items;
      });

  handler_.add<::lsp::requests::TextDocument_Definition>(
      [this](::lsp::requests::TextDocument_Definition::Params &&params)
          -> ::lsp::requests::TextDocument_Definition::Result {
        const std::string uri = std::string{params.textDocument.uri.path()};
        auto it = docs_.find(uri);
        if (it == docs_.end()) {
          return nullptr;
        }
        DefinitionProvider defp;
        auto loc = defp.definition(it->second, params.position);
        if (!loc) {
          return nullptr;
        }
        return *loc;
      });
}

} // namespace falcon::lsp
