// MIT License
//
// Copyright (c) Rei Shimizu 2020
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
//        of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
//        to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//        copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
//        copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//        AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef STARTEAR_TOKENIZER_H
#define STARTEAR_TOKENIZER_H

#include <iostream>
#include <memory>
#include <optional>
#include <queue>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "startear_assert.h"
#include "opcode.h"

namespace Startear {
enum TokenType {
  SEMICOLON,
  EQUAL,
  LEFT_PAREN,
  RIGHT_PAREN,
  LEFT_BRACE,
  RIGHT_BRACE,
  COMMA,
  DOT,
  MINUS,
  PLUS,
  STAR,
  BANG,
  BANG_EQUAL,
  EQUAL_EQUAL,
  GREATER,
  GREATER_EQUAL,
  LESS,
  LESS_EQUAL,
  SLASH,
  COMMENT,
  // ------ literals ------
  STRING,
  NUMBER,
  IDENTIFIER,
  // ------ reserved words -----
  VAR,
  FOR,
  FUN,
  TRUE,
  FALSE,
  IF,
  ELSE,
  NIL,
  RETURN
};

static std::unordered_map<TokenType, std::string> reserved_words{
    {TokenType::VAR, "let"},     {TokenType::FOR, "for"},
    {TokenType::FUN, "fn"},      {TokenType::TRUE, "true"},
    {TokenType::FALSE, "false"}, {TokenType::IF, "if"},
    {TokenType::ELSE, "else"},   {TokenType::NIL, "nil"}, {TokenType::RETURN, "return"}};

class Token {
 public:
  Token(TokenType type, std::string lexeme) : type_(type), lexeme_(lexeme) {}

  TokenType type() { return type_; }
  std::string lexeme() { return lexeme_; }

 private:
  TokenType type_;
  std::string lexeme_;
};

using TokenRef = std::reference_wrapper<Token>;

using TokenPtr = std::unique_ptr<Token>;

// == or !=
class Equality final : public Token {
 public:
  Equality(TokenType type, std::string lexeme) : Token(type, lexeme) {}
  Equality(Token& token) : Token(token.type(), token.lexeme()) {}
};

using EqualityPtr = std::unique_ptr<Equality>;

// <=, >=, <, >
class Compare final : public Token {
 public:
  Compare(TokenType type, std::string lexeme) : Token(type, lexeme) {}
  Compare(Token& token) : Token(token.type(), token.lexeme()) {}
};

using ComparePtr = std::unique_ptr<Compare>;

// +, -
class Addition final : public Token {
 public:
  Addition(TokenType type, std::string lexeme) : Token(type, lexeme) {}
  Addition(Token& token) : Token(token.type(), token.lexeme()) {}
};

using AdditionPtr = std::unique_ptr<Addition>;

// *, /
class Multiplication final : public Token {
 public:
  Multiplication(TokenType type, std::string lexeme) : Token(type, lexeme) {}
  Multiplication(Token& token) : Token(token.type(), token.lexeme()) {}
};

using MultiplicationPtr = std::unique_ptr<Multiplication>;

// !, -
class Unary final : public Token {
 public:
  Unary(TokenType type, std::string lexeme) : Token(type, lexeme) {}
  Unary(Token& token) : Token(token.type(), token.lexeme()) {}
};

using UnaryPtr = std::unique_ptr<Unary>;

// true/false, nil, literals
class Primary final : public Token {
 public:
  Primary(TokenType type, std::string lexeme) : Token(type, lexeme) {}
  Primary(Token& token) : Token(token.type(), token.lexeme()) {}
};

using PrimaryPtr = std::unique_ptr<Primary>;

// Normal tokens.
class Normal final : public Token {
 public:
  Normal(TokenType type, std::string lexeme) : Token(type, lexeme) {}
  Normal(Token& token) : Token(token.type(), token.lexeme()) {}
};

using NormalPtr = std::unique_ptr<Normal>;

class Tokenizer {
 public:
  Tokenizer(std::string code) : code_(code) {}
  std::vector<Token>& scanTokens();
  std::vector<Token>& scanTokens(std::string code);

 private:
  void scanToken();
  void addToken(Token&& token) { tokens_.emplace_back(token); }
  void parseInequality(TokenType t);
  void parseSlash();
  void parseString();
  void parseNumber();
  bool parseReservedWord(TokenType expected);
  void parseIdentifier();
  const char consume() {
    ++current_;
    return code_[current_ - 1];
  }

  bool nextMatch(char c) {
    ++current_;
    if (c == code_[current_ - 1]) {
      return true;
    } else {
      --current_;
      return false;
    }
  }
  bool isEnd() { return current_ >= code_.size(); }

  std::vector<Token> tokens_;
  size_t current_{0};
  std::string code_;
};

}  // namespace Startear
#endif  // STARTEAR_TOKENIZER_H
