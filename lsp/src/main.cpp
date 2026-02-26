#include "FalconServer.hpp"
#include <lsp/connection.h>
#include <lsp/io/standardio.h>
#include <lsp/messagehandler.h>
#include <iostream>

int main() {
    try {
        auto& io = lsp::io::standardIO();
        lsp::Connection connection(io);
        lsp::MessageHandler handler(connection);

        falcon::lsp::FalconServer server(handler);

        while (true) {
            handler.processIncomingMessages();
        }
    } catch (const lsp::ConnectionError&) {
        // Client disconnected — normal exit
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "falcon-lsp fatal error: " << e.what() << "\n";
        return 1;
    }
}
