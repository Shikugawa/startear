// MIT License
//
// Copyright (c) Rei Shimizu 2020
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
//        of this software and associated documentation files (the "Software"),
//        to deal
// in the Software without restriction, including without limitation the rights
//        to use, copy, modify, merge, publish, distribute, sublicense, and/or
//        sell copies of the Software, and to permit persons to whom the
//        Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all
//        copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//        AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "tokenizer.h"

namespace Startear {
namespace {
bool isDigit(char c) { return c >= '0' && c <= '9'; }

bool isAlpha(char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}
}  // namespace

std::vector<Token>& Tokenizer::scanTokens() {
  while (current_ < code_.size()) {
    scanToken();
  }
  return tokens_;
}

void Tokenizer::scanToken() {
  auto c = consume();
  switch (c) {
    case '"':
      parseString();
      break;
    case '+':
      addToken(Addition(TokenType::PLUS, "+"));
      break;
    case '-':
      addToken(Addition(TokenType::MINUS, "-"));
      break;
    case '*':
      addToken(Multiplication(TokenType::STAR, "*"));
      break;
    case '(':
      addToken(Normal(TokenType::LEFT_PAREN, "("));
      break;
    case ')':
      addToken(Normal(TokenType::RIGHT_PAREN, ")"));
      break;
    case '{':
      addToken(Normal(TokenType::LEFT_BRACE, "{"));
      break;
    case '}':
      addToken(Normal(TokenType::RIGHT_BRACE, "}"));
      break;
    case ',':
      addToken(Normal(TokenType::COMMA, ","));
      break;
    case '.':
      addToken(Normal(TokenType::DOT, "."));
      break;
    case ';':
      addToken(Normal(TokenType::SEMICOLON, ";"));
      break;
    case '!':
      parseInequality(TokenType::BANG);
      break;
    case '=':
      parseInequality(TokenType::EQUAL);
      break;
    case '>':
      parseInequality(TokenType::GREATER);
      break;
    case '<':
      parseInequality(TokenType::LESS);
      break;
    case '/':
      parseSlash();
      break;
    case 'l':
      if (parseReservedWord(TokenType::VAR)) break;
    case 'f':
      if (parseReservedWord(TokenType::FOR)) break;
      if (parseReservedWord(TokenType::FUN)) break;
    case 'r':
      if (parseReservedWord(TokenType::RETURN)) break;
    case 'i':
      if (parseReservedWord(TokenType::IF)) break;
    default:
      if (isDigit(c)) {
        parseNumber();
      } else if (isAlpha(c)) {
        parseIdentifier();
      }
      break;
  }
}

void Tokenizer::parseInequality(TokenType t) {
  bool is_next_equal = nextMatch('=');
  switch (t) {
    case TokenType::GREATER:
      is_next_equal ? addToken(Compare(TokenType::GREATER_EQUAL, ">="))
                    : addToken(Compare(TokenType::GREATER, ">"));
      return;
    case TokenType::LESS:
      is_next_equal ? addToken(Compare(TokenType::LESS_EQUAL, "<="))
                    : addToken(Compare(TokenType::LESS, "<"));
      return;
    case TokenType::EQUAL:
      is_next_equal ? addToken(Equality(TokenType::EQUAL_EQUAL, "=="))
                    : addToken(Normal(TokenType::EQUAL, "="));
      return;
    case TokenType::BANG:
      is_next_equal ? addToken(Equality(TokenType::BANG_EQUAL, "!="))
                    : addToken(Unary(TokenType::BANG, "!"));
      return;
    default:
      NOT_REACHED;
  }
}

void Tokenizer::parseSlash() {
  auto next_char = consume();

  if (next_char == '/') {
    std::string comment;
    while (!isEnd()) {
      next_char = consume();
      if (next_char == '\n') {
        break;
      }
      comment += next_char;
    }
    addToken(Normal(TokenType::COMMENT, comment));
  } else {
    --current_;
    addToken(Multiplication(TokenType::SLASH, "/"));
  }
}

void Tokenizer::parseString() {
  std::string data;
  bool terminated = false;
  while (!isEnd()) {
    auto next_char = consume();
    if (next_char == '"') {
      terminated = true;
      break;
    }
    data += next_char;
  }
  if (!terminated) {
    return;
  }
  addToken(Primary(TokenType::STRING, data));
}

void Tokenizer::parseNumber() {
  std::string number;
  number += code_[current_ - 1];
  while (!isEnd()) {
    auto next_char = consume();
    if (!isDigit(next_char) && next_char != '.') {
      --current_;
      break;
    }
    number += next_char;
  }
  addToken(Primary(TokenType::NUMBER, number));
}

bool Tokenizer::parseReservedWord(TokenType expected) {
  auto token_length = reserved_words[expected].size() - 1;
  auto proceed_counter = 0;
  std::string actual_token;
  actual_token += code_[current_ - 1];
  while (token_length > 0 && !isEnd()) {
    char next_char = consume();
    actual_token += next_char;
    --token_length;
    ++proceed_counter;
  }
  if (actual_token != reserved_words[expected]) {
    current_ -= proceed_counter;
    return false;
  } else {
    addToken(Normal(expected, reserved_words[expected]));
    return true;
  }
}

void Tokenizer::parseIdentifier() {
  std::string identifier;
  identifier += code_[current_ - 1];
  while (!isEnd()) {
    auto next_char = consume();
    if (!isAlpha(next_char) && !isDigit(next_char)) {
      --current_;
      break;
    }
    identifier += next_char;
  }

  addToken(Normal(TokenType::IDENTIFIER, identifier));
}
}  // namespace Startear