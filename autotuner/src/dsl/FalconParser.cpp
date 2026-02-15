#include "falcon-autotuner/dsl/FalconParser.hpp"
#include <sstream>
#include <stdexcept>

namespace falcon {
namespace autotuner {
namespace dsl {

FalconParser::FalconParser(std::vector<Token> tokens)
    : tokens_(std::move(tokens)) {}

std::vector<std::unique_ptr<AutotunerDecl>> FalconParser::parse() {
  std::vector<std::unique_ptr<AutotunerDecl>> autotuners;

  while (!at_end()) {
    if (match(TokenType::Autotuner)) {
      autotuners.push_back(parse_autotuner());
    } else {
      advance();
    }
  }

  return autotuners;
}

std::unique_ptr<AutotunerDecl> FalconParser::parse_autotuner() {
  auto autotuner = std::make_unique<AutotunerDecl>();

  // autotuner Name {
  autotuner->name = expect(TokenType::Identifier).value;
  expect(TokenType::LeftBrace);

  while (!match(TokenType::RightBrace) && !at_end()) {
    if (match(TokenType::Requires)) {
      expect(TokenType::Colon);
      autotuner->requirements = parse_requires();
    } else if (match(TokenType::Params)) {
      autotuner->parameters = parse_params();
    } else if (match(TokenType::Start)) {
      expect(TokenType::Arrow);
      autotuner->entry_state = expect(TokenType::Identifier).value;
      expect(TokenType::Semicolon);
    } else if (match(TokenType::State)) {
      autotuner->statements.push_back(parse_state());
    } else if (match(TokenType::For)) {
      autotuner->statements.push_back(parse_for_loop());
    } else if (match(TokenType::While)) {
      autotuner->statements.push_back(parse_while_loop());
    } else {
      advance(); // Skip unknown
    }
  }

  return autotuner;
}

std::vector<std::string> FalconParser::parse_requires() {
  std::vector<std::string> requirements;

  expect(TokenType::LeftBracket);

  while (!match(TokenType::RightBracket) && !at_end()) {
    requirements.push_back(expect(TokenType::Identifier).value);

    if (!match(TokenType::RightBracket)) {
      expect(TokenType::Comma);
    }
  }

  expect(TokenType::Semicolon);

  return requirements;
}

std::vector<std::unique_ptr<ParameterDecl>> FalconParser::parse_params() {
  std::vector<std::unique_ptr<ParameterDecl>> params;

  expect(TokenType::LeftBrace);

  while (!match(TokenType::RightBrace) && !at_end()) {
    auto param = std::make_unique<ParameterDecl>();

    // Type name = default_value;
    param->type = expect(TokenType::Identifier).value;
    param->name = expect(TokenType::Identifier).value;

    if (match(TokenType::Assign)) {
      Token value_token = advance();

      if (value_token.type == TokenType::IntLiteral) {
        param->default_value = std::stoll(value_token.value);
      } else if (value_token.type == TokenType::FloatLiteral) {
        param->default_value = std::stod(value_token.value);
      } else if (value_token.type == TokenType::StringLiteral) {
        param->default_value = value_token.value;
      } else if (value_token.value == "true") {
        param->default_value = true;
      } else if (value_token.value == "false") {
        param->default_value = false;
      }
    }

    expect(TokenType::Semicolon);
    params.push_back(std::move(param));
  }

  return params;
}

std::unique_ptr<StateDecl> FalconParser::parse_state() {
  auto state = std::make_unique<StateDecl>();

  // state name {
  state->name = expect(TokenType::Identifier).value;
  expect(TokenType::LeftBrace);

  while (!match(TokenType::RightBrace) && !at_end()) {
    if (match(TokenType::Params)) {
      expect(TokenType::LeftBrace);
      while (!match(TokenType::RightBrace) && !at_end()) {
        auto param = std::make_unique<ParameterDecl>();
        param->type = expect(TokenType::Identifier).value;
        param->name = expect(TokenType::Identifier).value;
        if (match(TokenType::Assign)) {
          Token value_token = advance();
          if (value_token.type == TokenType::IntLiteral) {
            param->default_value = std::stoll(value_token.value);
          } else if (value_token.type == TokenType::FloatLiteral) {
            param->default_value = std::stod(value_token.value);
          } else if (value_token.type == TokenType::StringLiteral) {
            param->default_value = value_token.value;
          } else if (value_token.value == "true") {
            param->default_value = true;
          } else if (value_token.value == "false") {
            param->default_value = false;
          }
        }
        expect(TokenType::Semicolon);
        state->state_params.push_back(std::move(param));
      }
    } else if (match(TokenType::Temp)) {
      expect(TokenType::LeftBrace);
      while (!match(TokenType::RightBrace) && !at_end()) {
        auto param = std::make_unique<ParameterDecl>();
        param->type = expect(TokenType::Identifier).value;
        param->name = expect(TokenType::Identifier).value;
        expect(TokenType::Semicolon);
        state->temp_params.push_back(std::move(param));
      }
    } else if (match(TokenType::Measurement)) {
      expect(TokenType::Colon);

      // Parse function call: func_name(args)
      std::string func_name = expect(TokenType::Identifier).value;
      state->measurement_call = func_name;

      expect(TokenType::LeftParen);

      // Parse arguments
      std::vector<std::string> args;
      while (!match(TokenType::RightParen) && !at_end()) {
        Token arg = advance();
        args.push_back(arg.value);

        if (!match(TokenType::RightParen)) {
          if (match(TokenType::Comma)) {
            // continue
          }
        }
      }

      // Store args in measurement call
      if (!args.empty()) {
        state->measurement_call += "(";
        for (size_t i = 0; i < args.size(); ++i) {
          if (i > 0)
            state->measurement_call += ", ";
          state->measurement_call += args[i];
        }
        state->measurement_call += ")";
      }

      expect(TokenType::Semicolon);
    } else if (match(TokenType::Terminal)) {
      state->is_terminal = true;
      expect(TokenType::Semicolon);
    } else if (match(TokenType::If) || peek().value == "else") {
      auto transitions = parse_transitions();
      for (auto &trans : transitions) {
        state->transitions.push_back(std::move(trans));
      }
    } else if (match(TokenType::Arrow)) {
      // Unconditional transition
      auto trans = std::make_unique<ConditionalTransition>();
      trans->condition = "true";

      std::string target = expect(TokenType::Identifier).value;

      // Handle :: for cross-autotuner
      if (match(TokenType::Colon) && match(TokenType::Colon)) {
        target += "::" + expect(TokenType::Identifier).value;
      }

      // Handle [var:mapped] syntax
      if (match(TokenType::LeftBracket)) {
        std::string transfer;
        while (!match(TokenType::RightBracket) && !at_end()) {
          transfer += advance().value;
        }
        if (!transfer.empty()) {
          target += "[" + transfer + "]";
        }
      }

      trans->target_state = target;
      expect(TokenType::Semicolon);
      state->transitions.push_back(std::move(trans));
    } else if (peek().type == TokenType::Identifier) {
      // Variable assignment: var = expr;
      std::string var_name = advance().value;
      expect(TokenType::Assign);

      // Parse expression
      std::string expr;
      while (!match(TokenType::Semicolon) && !at_end()) {
        expr += advance().value + " ";
      }

      // Store assignment (to be processed during code generation)
      if (state->transitions.empty() ||
          dynamic_cast<ConditionalTransition *>(
              state->transitions.back().get()) == nullptr) {
        auto trans = std::make_unique<ConditionalTransition>();
        state->transitions.push_back(std::move(trans));
      }

      auto *last_trans = dynamic_cast<ConditionalTransition *>(
          state->transitions.back().get());
      if (last_trans) {
        last_trans->assignments.push_back(var_name + " = " + expr);
      }
    } else {
      advance(); // Skip unknown
    }
  }

  return state;
}

std::vector<std::unique_ptr<ASTNode>> FalconParser::parse_transitions() {
  std::vector<std::unique_ptr<ASTNode>> transitions;

  while (peek().type == TokenType::If || peek().value == "else") {
    auto trans = parse_conditional_transition();
    transitions.push_back(std::move(trans));

    // Check if there's more (else if)
    if (peek().value != "else") {
      break;
    }
  }

  return transitions;
}

std::unique_ptr<ConditionalTransition>
FalconParser::parse_conditional_transition() {
  auto trans = std::make_unique<ConditionalTransition>();

  if (match(TokenType::If)) {
    // Parse condition
    expect(TokenType::LeftParen);

    std::string condition;
    int paren_depth = 1;
    while (paren_depth > 0 && !at_end()) {
      Token t = advance();
      if (t.type == TokenType::LeftParen)
        paren_depth++;
      if (t.type == TokenType::RightParen)
        paren_depth--;

      if (paren_depth > 0) {
        condition += t.value + " ";
      }
    }

    trans->condition = condition;

    // Optional: block of assignments
    if (match(TokenType::LeftBrace)) {
      while (!match(TokenType::RightBrace) && !at_end()) {
        if (peek().type == TokenType::Identifier) {
          std::string var = advance().value;
          expect(TokenType::Assign);
          std::string expr;
          while (!match(TokenType::Semicolon) && !at_end()) {
            expr += advance().value + " ";
          }
          trans->assignments.push_back(var + " = " + expr);
        } else {
          advance();
        }
      }
    }

    // Transition target
    if (match(TokenType::Arrow)) {
      std::string target = expect(TokenType::Identifier).value;

      // Handle :: for cross-autotuner
      if (match(TokenType::Colon) && match(TokenType::Colon)) {
        target += "::" + expect(TokenType::Identifier).value;
      }

      // Handle [var:mapped] syntax
      if (match(TokenType::LeftBracket)) {
        std::string transfer;
        while (!match(TokenType::RightBracket) && !at_end()) {
          transfer += advance().value;
        }
        if (!transfer.empty()) {
          target += "[" + transfer + "]";
        }
      }

      trans->target_state = target;
      expect(TokenType::Semicolon);
    }
  } else if (peek().value == "else") {
    advance(); // consume 'else'

    if (peek().type == TokenType::If) {
      // This is handled by the loop in parse_transitions
      return trans;
    } else {
      // else (unconditional)
      trans->condition = "true";

      // Optional block
      if (match(TokenType::LeftBrace)) {
        while (!match(TokenType::RightBrace) && !at_end()) {
          advance();
        }
      }

      if (match(TokenType::Arrow)) {
        std::string target = expect(TokenType::Identifier).value;

        if (match(TokenType::Colon) && match(TokenType::Colon)) {
          target += "::" + expect(TokenType::Identifier).value;
        }

        if (match(TokenType::LeftBracket)) {
          std::string transfer;
          while (!match(TokenType::RightBracket) && !at_end()) {
            transfer += advance().value;
          }
          if (!transfer.empty()) {
            target += "[" + transfer + "]";
          }
        }

        trans->target_state = target;
        expect(TokenType::Semicolon);
      }
    }
  }

  return trans;
}

std::unique_ptr<ForLoop> FalconParser::parse_for_loop() {
  auto loop = std::make_unique<ForLoop>();

  // for (var in expr) {
  expect(TokenType::LeftParen);
  loop->variable = expect(TokenType::Identifier).value;
  expect(TokenType::In);

  // Parse iterable expression
  std::string expr;
  while (!match(TokenType::RightParen) && !at_end()) {
    expr += advance().value;
  }
  loop->iterable_expr = expr;

  expect(TokenType::LeftBrace);

  // Parse body states
  while (!match(TokenType::RightBrace) && !at_end()) {
    if (match(TokenType::State)) {
      loop->body_states.push_back(parse_state());
    } else {
      advance();
    }
  }

  return loop;
}

std::unique_ptr<WhileLoop> FalconParser::parse_while_loop() {
  auto loop = std::make_unique<WhileLoop>();

  // while (condition) {
  expect(TokenType::LeftParen);

  std::string condition;
  int paren_depth = 1;
  while (paren_depth > 0 && !at_end()) {
    Token t = advance();
    if (t.type == TokenType::LeftParen)
      paren_depth++;
    if (t.type == TokenType::RightParen)
      paren_depth--;

    if (paren_depth > 0) {
      condition += t.value + " ";
    }
  }
  loop->condition = condition;

  expect(TokenType::LeftBrace);

  // Parse body states
  while (!match(TokenType::RightBrace) && !at_end()) {
    if (match(TokenType::State)) {
      loop->body_states.push_back(parse_state());
    } else {
      advance();
    }
  }

  return loop;
}

Token FalconParser::peek(int offset) const {
  size_t pos = position_ + offset;
  return pos < tokens_.size() ? tokens_[pos] : tokens_.back();
}

Token FalconParser::advance() {
  if (!at_end())
    position_++;
  return tokens_[position_ - 1];
}

bool FalconParser::match(TokenType type) {
  if (peek().type == type) {
    advance();
    return true;
  }
  return false;
}

Token FalconParser::expect(TokenType type) {
  if (peek().type != type) {
    throw std::runtime_error(
        "Expected token type " + std::to_string(static_cast<int>(type)) +
        " but got " + std::to_string(static_cast<int>(peek().type)) +
        " at line " + std::to_string(peek().line));
  }
  return advance();
}

bool FalconParser::at_end() const {
  return position_ >= tokens_.size() || peek().type == TokenType::EndOfFile;
}

} // namespace dsl
} // namespace autotuner
} // namespace falcon
