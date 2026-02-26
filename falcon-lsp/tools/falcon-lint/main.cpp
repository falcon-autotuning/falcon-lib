#include "falcon-atc/Compiler.hpp"
#include "falcon-atc/ParseError.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: falcon-lint <file.fal> [file2.fal ...]\n";
        return 1;
    }

    int exit_code = 0;
    for (int i = 1; i < argc; ++i) {
        falcon::atc::current_errors.clear();
        falcon::atc::Compiler compiler;
        try {
            compiler.parse_file(argv[i]);
            std::cout << argv[i] << ": OK\n";
        } catch (const std::exception& e) {
            std::cerr << e.what() << "\n";
            exit_code = 1;
        }
    }
    return exit_code;
}
