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

#ifndef STARTEAR_ALL_PARSER_H
#define STARTEAR_ALL_PARSER_H

#include <vector>

#include "ast.h"
#include "tokenizer.h"

namespace Startear {
class Parser {
 public:
  Parser(std::vector<Token>& tokens) : tokens_(tokens) {}

  ASTNodePtr parse();

 private:
  BasicExpressionPtr basicExpression();
  EqualityExpressionPtr equalityExpression();
  ComparisonExpressionPtr comparisonExpression();
  AdditionExpressionPtr additionExpression();
  MultiplicationExpressionPtr multiplicationExpression();
  UnaryExpressionPtr unaryExpression();
  PrimaryExpressionPtr primaryExpression();
  LetStatementPtr letStatement();
  FunctionDeclarationPtr functionDeclaration();
  ProgramDeclarationPtr programDeclaration();
  FunctionCallPtr functionCall();
  ReturnDeclarationPtr returnDeclaration();

  bool match(TokenType expected) { return match(expected, 0); }

  bool match(TokenType expected, size_t ahead) {
    if (ahead + current_ >= tokens_.size()) {
      return false;
    }
    return tokens_[current_ + ahead].type() == expected;
  }

  void forward() {
    if (isEnd()) {
      return;
    }
    ++current_;
  }

  bool isEnd() { return current_ >= tokens_.size(); }

  uint32_t current_{0};
  std::vector<Token> tokens_;
};
}  // namespace Startear

#endif  // STARTEAR_ALL_PARSER_H
