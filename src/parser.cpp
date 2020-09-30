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

#include "parser.h"

namespace Startear {

namespace {
bool isDigit(char c) { return c >= '0' && c <= '9'; }
}  // namespace

ASTNodePtr Parser::parse() { return programDeclaration(); }

BasicExpressionPtr Parser::basicExpression() {
  return std::make_unique<BasicExpression>(equalityExpression());
}

EqualityExpressionPtr Parser::equalityExpression() {
  auto left = comparisonExpression();
  EqualityExpressionPtr eql;

  while (match(TokenType::BANG_EQUAL) || match(TokenType::EQUAL_EQUAL)) {
    auto& root_token = tokens_[current_];
    forward();
    auto right = comparisonExpression();

    if (eql == nullptr) {
      eql = std::make_unique<EqualityExpression>(
          std::make_unique<Equality>(root_token), std::move(left),
          std::move(right));
    } else {
      auto new_eql = std::make_unique<EqualityExpression>(
          std::make_unique<Equality>(root_token), std::move(eql),
          std::move(right));
      eql = std::move(new_eql);
    }
  }

  if (eql == nullptr) {
    eql = std::make_unique<EqualityExpression>(std::move(left));
  }

  return eql;
}

ComparisonExpressionPtr Parser::comparisonExpression() {
  auto left = additionExpression();
  ComparisonExpressionPtr cmp;

  while (match(TokenType::GREATER) || match(TokenType::GREATER_EQUAL) ||
         match(TokenType::LESS_EQUAL) || match(TokenType::LESS)) {
    auto& root_token = tokens_[current_];
    forward();
    auto right = additionExpression();
    if (cmp == nullptr) {
      cmp = std::make_unique<ComparisonExpression>(
          std::make_unique<Compare>(root_token), std::move(left),
          std::move(right));
    } else {
      auto new_cmp = std::make_unique<ComparisonExpression>(
          std::make_unique<Compare>(root_token), std::move(cmp),
          std::move(right));
      cmp = std::move(new_cmp);
    }
  }
  if (cmp == nullptr) {
    cmp = std::make_unique<ComparisonExpression>(std::move(left));
  }
  return cmp;
}

AdditionExpressionPtr Parser::additionExpression() {
  auto left = multiplicationExpression();
  AdditionExpressionPtr add;

  while (match(TokenType::MINUS) || match(TokenType::PLUS)) {
    auto& root_token = tokens_[current_];
    forward();
    auto right = multiplicationExpression();

    if (add == nullptr) {
      add = std::make_unique<AdditionExpression>(
          std::make_unique<Addition>(root_token), std::move(left),
          std::move(right));
    } else {
      auto new_add = std::make_unique<AdditionExpression>(
          std::make_unique<Addition>(root_token), std::move(add),
          std::move(right));
      add = std::move(new_add);
    }
  }

  if (add == nullptr) {
    add = std::make_unique<AdditionExpression>(std::move(left));
  }

  return add;
}

MultiplicationExpressionPtr Parser::multiplicationExpression() {
  auto left = unaryExpression();
  MultiplicationExpressionPtr mul;

  while (match(TokenType::SLASH) || match(TokenType::STAR)) {
    auto root_token = tokens_[current_];
    forward();
    auto right = unaryExpression();

    if (mul == nullptr) {
      mul = std::make_unique<MultiplicationExpression>(
          std::make_unique<Multiplication>(root_token), std::move(left),
          std::move(right));
    } else {
      auto new_mul = std::make_unique<MultiplicationExpression>(
          std::make_unique<Multiplication>(root_token), std::move(mul),
          std::move(right));
      mul = std::move(new_mul);
    }
  }

  if (mul == nullptr) {
    mul = std::make_unique<MultiplicationExpression>(std::move(left));
  }

  return mul;
}

UnaryExpressionPtr Parser::unaryExpression() {
  auto root_token = tokens_[current_];
  if (match(TokenType::BANG) || match(TokenType::MINUS)) {
    forward();
    auto unary = unaryExpression();
    return std::make_unique<UnaryExpression>(
        std::make_unique<Unary>(root_token), std::move(unary));
  }

  auto primary = primaryExpression();
  return std::make_unique<UnaryExpression>(std::move(primary));
}

PrimaryExpressionPtr Parser::primaryExpression() {
  auto root_token = tokens_[current_];
  if (match(TokenType::NUMBER) || match(TokenType::STRING) ||
      match(TokenType::FALSE) || match(TokenType::TRUE) ||
      match(TokenType::NIL) ||
      match(TokenType::IDENTIFIER) /* To allow variable number */) {
    forward();
    return std::make_unique<PrimaryExpression>(
        std::make_unique<Primary>(root_token));
  } else if (match(TokenType::LEFT_PAREN)) {
    forward();
    auto expr = basicExpression();
    if (!match(TokenType::RIGHT_PAREN)) {
      std::cerr << "Parse Error" << std::endl;
      return nullptr;
    }
    forward();
    return std::make_unique<PrimaryExpression>(std::move(expr));
  }

  return nullptr;
}

LetStatementPtr Parser::letStatement(bool substitution) {
  // Not to pass `let` when passed statement is substitution.
  if (!substitution) {
    forward();
  }

  if (!match(TokenType::IDENTIFIER)) {
    return nullptr;
  }
  auto root_token = tokens_[current_];
  forward();
  if (match(TokenType::EQUAL)) {
    forward();
    LetStatementPtr stmt;
    if (match(TokenType::IDENTIFIER) && match(TokenType::LEFT_PAREN, 1)) {
      FunctionCallPtr expr = functionCall();
      if (!expr) {
        std::cerr << "failed to parse" << std::endl;
        return nullptr;
      }
      stmt = std::make_unique<LetStatement>(
          std::make_unique<Normal>(root_token), std::move(expr));
    } else {
      BasicExpressionPtr expr = basicExpression();
      if (!expr) {
        std::cerr << "failed to parse" << std::endl;
        return nullptr;
      }
      stmt = std::make_unique<LetStatement>(
          std::make_unique<Normal>(root_token), std::move(expr));
    }
    return stmt;
  }
  return nullptr;
}

FunctionCallPtr Parser::functionCall() {
  auto name_token = tokens_[current_];
  forward();
  if (!match(TokenType::LEFT_PAREN)) {
    std::cerr << "Missing (" << std::endl;
    return nullptr;
  }
  forward();
  std::vector<BasicExpressionPtr> stmts;
  while (!match(TokenType::SEMICOLON)) {
    auto basic_stmt = basicExpression();
    stmts.emplace_back(std::move(basic_stmt));
    if (match(TokenType::COMMA)) {
      forward();
    } else if (match(TokenType::RIGHT_PAREN)) {
      forward();
    } else {
      NOT_REACHED;
    }
  }
  return std::make_unique<FunctionCall>(std::make_unique<Normal>(name_token),
                                        stmts);
}

ReturnDeclarationPtr Parser::returnDeclaration() {
  forward();
  auto return_value_token = tokens_[current_];
  forward();
  bool is_literal = true;
  if (return_value_token.lexeme()[0] != '"') {  // string literal or not
    for (const auto& ch : return_value_token.lexeme()) {
      if (!isDigit(ch)) {  // if true it must be variable
        is_literal = false;
        break;
      }
    }
  }
  if (is_literal) {
    return std::make_unique<ReturnDeclaration>(
        std::make_unique<Primary>(return_value_token));
  }
  return std::make_unique<ReturnDeclaration>(
      std::make_unique<Normal>(return_value_token));
}

IfStatementPtr Parser::ifStatement() {
  forward();
  if (!match(TokenType::LEFT_PAREN)) {
    // TODO: add line no
    std::cerr << "Syntax Error" << std::endl;
    NOT_REACHED;
  }
  forward();
  auto eql_expr_ptr = equalityExpression();
  if (!match(TokenType::RIGHT_PAREN)) {
    std::cerr << "Syntax Error" << std::endl;
    NOT_REACHED;
  }
  forward();
  if (!match(TokenType::LEFT_BRACE)) {
    std::cerr << "Syntax Error" << std::endl;
    NOT_REACHED;
  }
  forward();
  std::vector<ASTNodePtr> expressions;
  while (!match(TokenType::RIGHT_BRACE)) {
    if (match(TokenType::RIGHT_BRACE)) {
      forward();
      break;
    }
    ASTNodePtr current_stmt;
    if (match(TokenType::VAR)) {
      current_stmt = letStatement();
    } else if (match(TokenType::COMMENT)) {
      forward();
      continue;
    } else if (match(TokenType::IDENTIFIER)) {
      current_stmt = functionCall();
    } else if (match(TokenType::RETURN)) {
      current_stmt = returnDeclaration();
    } else {
      NOT_REACHED;
    }
    if (!current_stmt) {
      // TODO: line number
      std::cerr << "Failed to parse" << std::endl;
      return nullptr;
    }
    if (!match(TokenType::SEMICOLON)) {
      std::cout << "Variable definition must be ended with semicolon"
                << std::endl;
      return nullptr;
    }
    forward();
    expressions.emplace_back(std::move(current_stmt));
  }
  forward();
  return std::make_unique<IfStatement>(std::move(eql_expr_ptr), expressions);
}

FunctionDeclarationPtr Parser::functionDeclaration() {
  forward();
  auto name_token = tokens_[current_];
  forward();
  if (!match(TokenType::LEFT_PAREN)) {
    std::cerr << "Missing (" << std::endl;
    return nullptr;
  }
  forward();
  std::vector<TokenPtr> args;
  if (match(TokenType::IDENTIFIER)) {
    // TODO: Should alert if there is no termination symbol of function.
    while (!isEnd()) {
      auto current_token = tokens_[current_];
      forward();
      if (match(TokenType::RIGHT_PAREN)) {
        args.emplace_back(std::make_unique<Normal>(current_token));
        forward();
        break;
      } else if (!match(TokenType::COMMA)) {
        std::cerr << "arguments should be separated by comma" << std::endl;
        return nullptr;
      }
      args.emplace_back(std::make_unique<Normal>(current_token));
      forward();
    }
    if (!match(TokenType::LEFT_BRACE)) {
      std::cerr << "function should be started with left bracket" << std::endl;
      return nullptr;
    }
  } else if (match(TokenType::RIGHT_PAREN)) {
    forward();
  } else {
    NOT_REACHED;
  }
  forward();
  std::vector<ASTNodePtr> expressions;
  while (!isEnd()) {
    if (match(TokenType::RIGHT_BRACE)) {
      forward();
      break;
    }
    ASTNodePtr current_stmt;
    if (match(TokenType::VAR)) {
      current_stmt = letStatement();
    } else if (match(TokenType::COMMENT)) {
      forward();
      continue;
    } else if (match(TokenType::IDENTIFIER)) {
      if (match(TokenType::LEFT_PAREN, 1)) {
        current_stmt = functionCall();
      } else if (match(TokenType::EQUAL, 1)) {
        current_stmt = letStatement(true);
      } else {
        NOT_REACHED;
      }
    } else if (match(TokenType::RETURN)) {
      current_stmt = returnDeclaration();
    } else if (match(TokenType::IF)) {
      current_stmt = ifStatement();
    } else {
      // TODO: Show line number
      std::cerr << "Syntax Error" << std::endl;
      NOT_REACHED;
    }
    if (!current_stmt) {
      // TODO: line number
      std::cerr << "Failed to parse" << std::endl;
      return nullptr;
    }
    if (!match(TokenType::SEMICOLON)) {
      std::cout << "Variable definition must be ended with semicolon"
                << std::endl;
      return nullptr;
    }
    forward();
    expressions.emplace_back(std::move(current_stmt));
  }
  return std::make_unique<FunctionDeclaration>(
      std::make_unique<Normal>(name_token), args, expressions);
}

ProgramDeclarationPtr Parser::programDeclaration() {
  std::vector<LetStatementPtr> let_statements;
  std::vector<FunctionDeclarationPtr> func_decls;
  std::vector<BasicExpressionPtr> basic_exprs;
  while (!isEnd()) {
    if (match(TokenType::VAR)) {
      auto let_stmt = letStatement();
      if (!let_stmt) {
        return nullptr;
      }
      let_statements.emplace_back(std::move(let_stmt));
    } else if (match(TokenType::FUN)) {
      auto func_decl = functionDeclaration();
      if (!func_decl) {
        return nullptr;
      }
      func_decls.emplace_back(std::move(func_decl));
    } else if (match(TokenType::COMMENT)) {
      forward();
      continue;
    } else {
      // In general, this section is not reached except testing.
      auto basic_expr = basicExpression();
      if (!basic_expr) {
        return nullptr;
      }
      basic_exprs.emplace_back(std::move(basic_expr));
    }
  }
  return std::make_unique<ProgramDeclaration>(let_statements, func_decls,
                                              basic_exprs);
}

}  // namespace Startear
