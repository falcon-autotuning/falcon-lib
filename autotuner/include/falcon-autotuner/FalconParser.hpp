#pragma once

#include <filesystem>
#include <memory>
#include <set>
#include <string>
#include <variant>
#include <vector>

namespace falcon::autotuner::dsl {

/**
 * @brief Token types for Falcon language
 */
enum class TokenType : std::uint8_t {
  // Keywords
  Autotuner,
  Requires,
  Params,
  State,
  Measurement,
  If,
  Else,
  For,
  While,
  In,
  Range,
  Terminal,
  Start,
  Temp,

  // Symbols
  LeftBrace,
  RightBrace,
  LeftParen,
  RightParen,
  LeftBracket,
  RightBracket,
  Arrow,
  Semicolon,
  Comma,
  Colon,
  Dollar,
  Dot,

  // Operators
  Plus,
  Minus,
  Multiply,
  Divide,
  Modulo,
  Equal,
  NotEqual,
  Less,
  Greater,
  LessEqual,
  GreaterEqual,
  And,
  Or,
  Not,
  Assign,

  // Literals
  Identifier,
  IntLiteral,
  FloatLiteral,
  StringLiteral,

  // Special
  EndOfFile,
  Unknown
};

struct Token {
  TokenType type;
  std::string value;
  int line;
  int column;
};

/**
 * @brief AST Node types
 */
struct ASTNode {
  virtual ~ASTNode() = default;
};

struct ParameterDecl : ASTNode {
  std::string type; // "int", "float", "string", "bool"
  std::string name;
  std::variant<int64_t, double, std::string, bool> default_value;
};

struct StateDecl : ASTNode {
  std::string name;
  std::vector<std::unique_ptr<ParameterDecl>> state_params; // params {}
  std::vector<std::unique_ptr<ParameterDecl>> temp_params;  // temp {}
  std::string measurement_call; // e.g., "measure_voltage(voltage)"
  std::vector<std::unique_ptr<ASTNode>> transitions;
  bool is_terminal = false;
};

struct ConditionalTransition : ASTNode {
  std::string condition; // Expression as string
  std::string
      target_state; // Can be "AutotunerName::state_name" or "state[var:mapped]"
  std::vector<std::string> assignments; // e.g., "x = x + 1"
};

struct ForLoop : ASTNode {
  std::string variable;
  std::string iterable_expr; // e.g., "range(0, 10, 1)" or "voltage_list"
  std::vector<std::unique_ptr<StateDecl>> body_states;
};

struct WhileLoop : ASTNode {
  std::string condition;
  std::vector<std::unique_ptr<StateDecl>> body_states;
};

struct AutotunerDecl : ASTNode {
  std::string name;
  std::vector<std::string> requirements; // Other autotuners or libraries
  std::vector<std::unique_ptr<ParameterDecl>> parameters;
  std::string entry_state;
  std::vector<std::unique_ptr<ASTNode>> statements; // States, loops, etc.
};

/**
 * @brief Lexer for Falcon language
 */
class FalconLexer {
public:
  explicit FalconLexer(const std::string &source);

  std::vector<Token> tokenize();

private:
  Token next_token();
  [[nodiscard]] char peek(int offset = 0) const;
  char advance();
  void skip_whitespace();
  void skip_comment();
  Token read_identifier();
  Token read_number();
  Token read_string();

  std::string source_;
  size_t position_ = 0;
  int line_ = 1;
  int column_ = 1;
};

/**
 * @brief Parser for Falcon language
 */
class FalconParser {
public:
  explicit FalconParser(std::vector<Token> tokens);

  std::vector<std::unique_ptr<AutotunerDecl>> parse();

private:
  std::unique_ptr<AutotunerDecl> parse_autotuner();
  std::vector<std::string> parse_requires();
  std::vector<std::unique_ptr<ParameterDecl>> parse_params();
  std::unique_ptr<StateDecl> parse_state();
  std::unique_ptr<ForLoop> parse_for_loop();
  std::unique_ptr<WhileLoop> parse_while_loop();
  std::vector<std::unique_ptr<ASTNode>> parse_transitions();
  std::unique_ptr<ConditionalTransition> parse_conditional_transition();

  [[nodiscard]] Token peek(int offset = 0) const;
  Token advance();
  bool match(TokenType type);
  Token expect(TokenType type);
  [[nodiscard]] bool at_end() const;

  std::vector<Token> tokens_;
  size_t position_ = 0;
};

/**
 * @brief Code generator - generates C++ from AST
 */
class FalconCodeGenerator {
public:
  struct GeneratedCode {
    std::string header_code;                        // .hpp file
    std::string source_code;                        // .cpp file
    std::vector<std::string> required_measurements; // Functions to implement
  };

  GeneratedCode
  generate(const std::vector<std::unique_ptr<AutotunerDecl>> &ast);

private:
  void generate_autotuner_builder(const AutotunerDecl &autotuner,
                                  std::ostringstream &header,
                                  std::ostringstream &source,
                                  GeneratedCode &code);

  void generate_statement(const ASTNode &stmt, const AutotunerDecl &autotuner,
                          std::ostringstream &source);

  void generate_state_config(const StateDecl &state,
                             const AutotunerDecl &autotuner,
                             std::ostringstream &source);

  void generate_transition_with_transfer(const ConditionalTransition &trans,
                                         const std::string &state_var,
                                         const AutotunerDecl &autotuner,
                                         std::ostringstream &source);

  void generate_for_loop(const ForLoop &loop, std::ostringstream &source);

  void generate_while_loop(const WhileLoop &loop, std::ostringstream &source);

  std::string generate_condition_code(const std::string &condition);

  std::set<std::string> collect_state_names(const AutotunerDecl &autotuner);
  std::set<std::string> extract_measurements(const AutotunerDecl &autotuner);
  std::string sanitize_name(const std::string &name);
  std::string trim(const std::string &str);
  std::string substitute_variable(const std::string &text,
                                  const std::string &variable);
};

struct CompilerOptions {
  std::filesystem::path output_dir = "generated";
  bool generate_documentation = true;
  bool validate_dependencies = true;
};
/**
 * @brief Main compiler interface
 */
class FalconCompiler {
public:
  explicit FalconCompiler(CompilerOptions options = {});

  /**
   * @brief Compile a .fal file to C++
   */
  void compile_file(const std::filesystem::path &fal_file);

  /**
   * @brief Compile all .fal files in a directory
   */
  void compile_directory(const std::filesystem::path &directory);

  /**
   * @brief Get list of measurement functions that need to be implemented
   */
  [[nodiscard]] std::vector<std::string> get_required_implementations() const;

private:
  CompilerOptions options_;
  std::vector<std::string> required_implementations_;
};

} // namespace falcon::autotuner::dsl
