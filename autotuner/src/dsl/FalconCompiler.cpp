#include "falcon-autotuner/dsl/FalconParser.hpp"
#include <fstream>
#include <iostream>

namespace falcon {
namespace autotuner {
namespace dsl {

FalconCompiler::FalconCompiler(CompilerOptions options)
    : options_(std::move(options)) {}

void FalconCompiler::compile_file(const std::filesystem::path &fal_file) {
  std::cout << "Compiling: " << fal_file << std::endl;

  // Read source file
  std::ifstream file(fal_file);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + fal_file.string());
  }

  std::string source((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
  file.close();

  // Lex
  FalconLexer lexer(source);
  auto tokens = lexer.tokenize();

  std::cout << "  Lexed " << tokens.size() << " tokens" << std::endl;

  // Parse
  FalconParser parser(std::move(tokens));
  auto ast = parser.parse();

  std::cout << "  Parsed " << ast.size() << " autotuner(s)" << std::endl;

  // Generate code
  FalconCodeGenerator generator;
  auto generated = generator.generate(ast);

  // Store required implementations
  required_implementations_.insert(required_implementations_.end(),
                                   generated.required_measurements.begin(),
                                   generated.required_measurements.end());

  // Write output files
  std::filesystem::create_directories(options_.output_dir);

  std::ofstream header_file(options_.output_dir / "GeneratedAutotuners.hpp");
  if (!header_file.is_open()) {
    throw std::runtime_error("Failed to create header file");
  }
  header_file << generated.header_code;
  header_file.close();

  std::ofstream source_file(options_.output_dir / "GeneratedAutotuners.cpp");
  if (!source_file.is_open()) {
    throw std::runtime_error("Failed to create source file");
  }
  source_file << generated.source_code;
  source_file.close();

  std::cout << "  Generated:" << std::endl;
  std::cout << "    "
            << (options_.output_dir / "GeneratedAutotuners.hpp").string()
            << std::endl;
  std::cout << "    "
            << (options_.output_dir / "GeneratedAutotuners.cpp").string()
            << std::endl;
}

void FalconCompiler::compile_directory(const std::filesystem::path &directory) {
  for (const auto &entry : std::filesystem::directory_iterator(directory)) {
    if (entry.path().extension() == ".fal") {
      try {
        compile_file(entry.path());
      } catch (const std::exception &e) {
        std::cerr << "Error compiling " << entry.path() << ": " << e.what()
                  << std::endl;
      }
    }
  }
}

std::vector<std::string> FalconCompiler::get_required_implementations() const {
  return required_implementations_;
}

} // namespace dsl
} // namespace autotuner
} // namespace falcon
