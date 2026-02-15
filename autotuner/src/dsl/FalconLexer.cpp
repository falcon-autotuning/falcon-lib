#include "falcon-autotuner/dsl/FalconParser.hpp"
#include <cctype>
#include <stdexcept>

namespace falcon {
namespace autotuner {
namespace dsl {

FalconLexer::FalconLexer(const std::string &source) : source_(source) {}

std::vector<Token> FalconLexer::tokenize() {
  std::vector<Token> tokens;

  while (position_ < source_.length()) {
    skip_whitespace();

    if (position_ >= source_.length())
      break;

    // Skip comments
    if (peek() == '/' && peek(1) == '/') {
      skip_comment();
      continue;
    }

    Token token = next_token();
    tokens.push_back(token);

    if (token.type == TokenType::Unknown) {
      throw std::runtime_error("Unknown token at line " +
                               std::to_string(line_) + ", column " +
                               std::to_string(column_));
    }
  }

  tokens.push_back(Token{TokenType::EndOfFile, "", line_, column_});
  return tokens;
}

char FalconLexer::peek(int offset) const {
  size_t pos = position_ + offset;
  return pos < source_.length() ? source_[pos] : '\0';
}

char FalconLexer::advance() {
  char c = source_[position_++];
  if (c == '\n') {
    line_++;
    column_ = 1;
  } else {
    column_++;
  }
  return c;
}

void FalconLexer::skip_whitespace() {
  while (position_ < source_.length() && std::isspace(peek())) {
    advance();
  }
}

void FalconLexer::skip_comment() {
  while (position_ < source_.length() && peek() != '\n') {
    advance();
  }
}

Token FalconLexer::next_token() {
  char c = peek();

  // Identifiers and keywords
  if (std::isalpha(c) || c == '_') {
    return read_identifier();
  }

  // Numbers
  if (std::isdigit(c) || (c == '-' && std::isdigit(peek(1)))) {
    return read_number();
  }

  // Strings
  if (c == '"') {
    return read_string();
  }

  // Two-character operators
  if (c == '-' && peek(1) == '>') {
    int line = line_, col = column_;
    advance();
    advance();
    return Token{TokenType::Arrow, "->", line, col};
  }

  if (c == '=' && peek(1) == '=') {
    int line = line_, col = column_;
    advance();
    advance();
    return Token{TokenType::Equal, "==", line, col};
  }

  if (c == '!' && peek(1) == '=') {
    int line = line_, col = column_;
    advance();
    advance();
    return Token{TokenType::NotEqual, "!=", line, col};
  }

  if (c == '<' && peek(1) == '=') {
    int line = line_, col = column_;
    advance();
    advance();
    return Token{TokenType::LessEqual, "<=", line, col};
  }

  if (c == '>' && peek(1) == '=') {
    int line = line_, col = column_;
    advance();
    advance();
    return Token{TokenType::GreaterEqual, ">=", line, col};
  }

  if (c == '&' && peek(1) == '&') {
    int line = line_, col = column_;
    advance();
    advance();
    return Token{TokenType::And, "&&", line, col};
  }

  if (c == '|' && peek(1) == '|') {
    int line = line_, col = column_;
    advance();
    advance();
    return Token{TokenType::Or, "||", line, col};
  }

  // Single-character tokens
  int line = line_, col = column_;
  advance();

  switch (c) {
  case '{':
    return Token{TokenType::LeftBrace, "{", line, col};
  case '}':
    return Token{TokenType::RightBrace, "}", line, col};
  case '(':
    return Token{TokenType::LeftParen, "(", line, col};
  case ')':
    return Token{TokenType::RightParen, ")", line, col};
  case '[':
    return Token{TokenType::LeftBracket, "[", line, col};
  case ']':
    return Token{TokenType::RightBracket, "]", line, col};
  case ';':
    return Token{TokenType::Semicolon, ";", line, col};
  case ',':
    return Token{TokenType::Comma, ",", line, col};
  case ':':
    return Token{TokenType::Colon, ":", line, col};
  case '$':
    return Token{TokenType::Dollar, "$", line, col};
  case '.':
    return Token{TokenType::Dot, ".", line, col};
  case '+':
    return Token{TokenType::Plus, "+", line, col};
  case '-':
    return Token{TokenType::Minus, "-", line, col};
  case '*':
    return Token{TokenType::Multiply, "*", line, col};
  case '/':
    return Token{TokenType::Divide, "/", line, col};
  case '%':
    return Token{TokenType::Modulo, "%", line, col};
  case '=':
    return Token{TokenType::Assign, "=", line, col};
  case '<':
    return Token{TokenType::Less, "<", line, col};
  case '>':
    return Token{TokenType::Greater, ">", line, col};
  case '!':
    return Token{TokenType::Not, "!", line, col};
  default:
    return Token{TokenType::Unknown, std::string(1, c), line, col};
  }
}

Token FalconLexer::read_identifier() {
  int line = line_, col = column_;
  std::string value;

  while (position_ < source_.length() &&
         (std::isalnum(peek()) || peek() == '_')) {
    value += advance();
  }

  // Check for keywords
  static const std::map<std::string, TokenType> keywords = {
      {"autotuner", TokenType::Autotuner},
      {"requires", TokenType::Requires},
      {"params", TokenType::Params},
      {"state", TokenType::State},
      {"measurement", TokenType::Measurement},
      {"if", TokenType::If},
      {"else", TokenType::Else},
      {"for", TokenType::For},
      {"while", TokenType::While},
      {"in", TokenType::In},
      {"range", TokenType::Range},
      {"terminal", TokenType::Terminal},
      {"start", TokenType::Start},
      {"true", TokenType::Identifier}, // Treat as identifier
      {"false", TokenType::Identifier},
  };

  auto it = keywords.find(value);
  TokenType type = (it != keywords.end()) ? it->second : TokenType::Identifier;

  return Token{type, value, line, col};
}

Token FalconLexer::read_number() {
  int line = line_, col = column_;
  std::string value;
  bool is_float = false;

  if (peek() == '-') {
    value += advance();
  }

  while (position_ < source_.length() &&
         (std::isdigit(peek()) || peek() == '.' || peek() == 'e' ||
          peek() == 'E')) {
    char c = peek();
    if (c == '.')
      is_float = true;
    value += advance();

    if (c == 'e' || c == 'E') {
      is_float = true;
      if (peek() == '+' || peek() == '-') {
        value += advance();
      }
    }
  }

  TokenType type = is_float ? TokenType::FloatLiteral : TokenType::IntLiteral;
  return Token{type, value, line, col};
}

Token FalconLexer::read_string() {
  int line = line_, col = column_;
  std::string value;

  advance(); // Skip opening quote

  while (position_ < source_.length() && peek() != '"') {
    if (peek() == '\\' && peek(1) == '"') {
      advance();
      value += advance();
    } else {
      value += advance();
    }
  }

  if (peek() == '"') {
    advance(); // Skip closing quote
  }

  return Token{TokenType::StringLiteral, value, line, col};
}

} // namespace dsl
} // namespace autotuner
} // namespace falcon
