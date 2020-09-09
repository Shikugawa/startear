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

#include "ast.h"

#include "fmt/format.h"

namespace Startear {

std::string PrimaryExpression::toString() {
  if (expr_ != nullptr) {
    return fmt::format("{}", expr_->toString());
  }
  if (token_ != nullptr) {
    return fmt::format("{}", token_->lexeme());
  }
}

void PrimaryExpression::accept(IASTNodeVisitor& visitor) {
  visitor.visit(*this);
}

void PrimaryExpression::self(Program& program) {
  if (expr_ != nullptr) {
    static_cast<ASTNode*>(expr_.get())->self(program);
  } else if (token_ != nullptr) {
    if (token_->type() == TokenType::NUMBER) {
      program.addInst(OPCode::OP_PUSH,
                      {std::make_pair(Value::Category::Literal,
                                      std::stod(token_->lexeme()))});
    } else if (token_->type() == TokenType::IDENTIFIER) {
      program.addInst(
          OPCode::OP_GET_LOCAL,
          {std::make_pair(Value::Category::Variable, token_->lexeme())});
    } else {
      NOT_REACHED;
    }
  } else {
    NOT_REACHED;
  }
}

void UnaryExpression::accept(IASTNodeVisitor& visitor) { visitor.visit(*this); }

std::string UnaryExpression::toString() {
  if (primary_expr_ != nullptr) {
    return fmt::format("{}", primary_expr_->toString());
  }
  if (unary_expr_ != nullptr && !token_->lexeme().empty()) {
    return fmt::format("({} {})", token_->lexeme(), unary_expr_->toString());
  }
  NOT_REACHED;
}

void UnaryExpression::self(Program& program) {
  if (primary_expr_ != nullptr) {
    static_cast<ASTNode*>(primary_expr_.get())->self(program);
  } else if (unary_expr_ != nullptr && !token_->lexeme().empty()) {
    static_cast<ASTNode*>(unary_expr_.get())->self(program);
  } else {
    NOT_REACHED;
  }
}

std::string MultiplicationExpression::toString() {
  if (unary_left_expr_ != nullptr && right_expr_ != nullptr) {
    return fmt::format("({} {} {})", token_->lexeme(),
                       unary_left_expr_->toString(), right_expr_->toString());
  }
  if (mul_left_expr_ != nullptr && right_expr_ != nullptr) {
    return fmt::format("({} {} {})", token_->lexeme(),
                       mul_left_expr_->toString(), right_expr_->toString());
  }
  if (unary_left_expr_ != nullptr) {
    return fmt::format("{}", unary_left_expr_->toString());
  }
  NOT_REACHED;
}

void MultiplicationExpression::accept(IASTNodeVisitor& visitor) {
  visitor.visit(*this);
}

void MultiplicationExpression::self(Program& program) {
  if (unary_left_expr_ != nullptr && right_expr_ != nullptr) {
    static_cast<ASTNode*>(unary_left_expr_.get())->self(program);
    static_cast<ASTNode*>(right_expr_.get())->self(program);
  } else if (mul_left_expr_ != nullptr && right_expr_ != nullptr) {
    static_cast<ASTNode*>(mul_left_expr_.get())->self(program);
    static_cast<ASTNode*>(right_expr_.get())->self(program);
  } else if (unary_left_expr_ != nullptr) {
    static_cast<ASTNode*>(unary_left_expr_.get())->self(program);
  } else {
    NOT_REACHED;
  }
}

std::string AdditionExpression::toString() {
  if (add_left_expr_ != nullptr && right_expr_ != nullptr) {
    return fmt::format("({} {} {})", token_->lexeme(),
                       add_left_expr_->toString(), right_expr_->toString());
  }
  if (mul_left_expr_ != nullptr && right_expr_ != nullptr) {
    return fmt::format("({} {} {})", token_->lexeme(),
                       mul_left_expr_->toString(), right_expr_->toString());
  }
  if (mul_left_expr_ != nullptr) {
    return fmt::format("{}", mul_left_expr_->toString());
  }
  NOT_REACHED;
}

void AdditionExpression::accept(IASTNodeVisitor& visitor) {
  visitor.visit(*this);
}

void AdditionExpression::self(Program& program) {
  if (add_left_expr_ != nullptr && right_expr_ != nullptr) {
    static_cast<ASTNode*>(add_left_expr_.get())->self(program);
    static_cast<ASTNode*>(right_expr_.get())->self(program);
    program.addInst(OPCode::OP_ADD);
  } else if (mul_left_expr_ != nullptr && right_expr_ != nullptr) {
    static_cast<ASTNode*>(mul_left_expr_.get())->self(program);
    static_cast<ASTNode*>(right_expr_.get())->self(program);
    program.addInst(OPCode::OP_ADD);
  } else if (mul_left_expr_ != nullptr) {
    static_cast<ASTNode*>(mul_left_expr_.get())->self(program);
  } else {
    NOT_REACHED;
  }
}

std::string ComparisonExpression::toString() {
  if (cmp_left_expr_ != nullptr && right_expr_ != nullptr) {
    return fmt::format("({} {} {})", token_->lexeme(),
                       cmp_left_expr_->toString(), right_expr_->toString());
  }
  if (add_left_expr_ != nullptr && right_expr_ != nullptr) {
    return fmt::format("({} {} {})", token_->lexeme(),
                       cmp_left_expr_->toString(), right_expr_->toString());
  }
  if (add_left_expr_ != nullptr) {
    return fmt::format("{}", add_left_expr_->toString());
  }
  NOT_REACHED;
}

void ComparisonExpression::accept(IASTNodeVisitor& visitor) {
  visitor.visit(*this);
}

void ComparisonExpression::self(Program& program) {
  if (cmp_left_expr_ != nullptr && right_expr_ != nullptr) {
    static_cast<ASTNode*>(cmp_left_expr_.get())->self(program);
    static_cast<ASTNode*>(right_expr_.get())->self(program);
  } else if (add_left_expr_ != nullptr && right_expr_ != nullptr) {
    static_cast<ASTNode*>(add_left_expr_.get())->self(program);
    static_cast<ASTNode*>(right_expr_.get())->self(program);
  } else if (add_left_expr_ != nullptr) {
    static_cast<ASTNode*>(add_left_expr_.get())->self(program);
  } else {
    NOT_REACHED;
  }
}

std::string EqualityExpression::toString() {
  if (eql_left_expr_ != nullptr && right_expr_ != nullptr) {
    return fmt::format("({} {} {})", token_->lexeme(),
                       eql_left_expr_->toString(), right_expr_->toString());
  }
  if (cmp_left_expr_ != nullptr && right_expr_ != nullptr) {
    return fmt::format("({} {} {})", token_->lexeme(),
                       cmp_left_expr_->toString(), right_expr_->toString());
  }
  if (cmp_left_expr_ != nullptr) {
    return fmt::format("{}", cmp_left_expr_->toString());
  }
  NOT_REACHED;
}

void EqualityExpression::accept(IASTNodeVisitor& visitor) {
  visitor.visit(*this);
}

void EqualityExpression::self(Program& program) {
  if (eql_left_expr_ != nullptr && right_expr_ != nullptr) {
    static_cast<ASTNode*>(eql_left_expr_.get())->self(program);
    static_cast<ASTNode*>(right_expr_.get())->self(program);
  } else if (cmp_left_expr_ != nullptr && right_expr_ != nullptr) {
    static_cast<ASTNode*>(cmp_left_expr_.get())->self(program);
    static_cast<ASTNode*>(right_expr_.get())->self(program);
  } else if (cmp_left_expr_ != nullptr) {
    static_cast<ASTNode*>(cmp_left_expr_.get())->self(program);
  } else {
    NOT_REACHED;
  }
}

std::string BasicExpression::toString() { return expr_->toString(); }

void BasicExpression::accept(IASTNodeVisitor& visitor) { visitor.visit(*this); }

void BasicExpression::self(Program& program) {
  static_cast<ASTNode*>(expr_.get())->self(program);
}

std::string LetStatement::toString() {
  return fmt::format("{} -> {}", token_->lexeme(), expr_->toString());
}

void LetStatement::accept(IASTNodeVisitor& visitor) { visitor.visit(*this); }

void LetStatement::self(Program& program) {
  static_cast<ASTNode*>(expr_.get())->self(program);
  program.addInst(OPCode::OP_STORE_LOCAL,
                  {std::make_pair(Value::Category::Literal, token_->lexeme())});
}

void FunctionCall::accept(IASTNodeVisitor& visitor) { visitor.visit(*this); }

void FunctionCall::self(Program& program) {
  program.addInst(OPCode::OP_PUSH_FRAME);
  for (const auto& stmt : statements_) {
    static_cast<ASTNode*>(stmt.get())->self(program);
  }
  program.addInst(OPCode::OP_CALL, {std::make_pair(Value::Category::Variable,
                                                   token_->lexeme())});
}

std::string FunctionCall::toString() {
  std::string str;
  str += fmt::format("{} (", token_->lexeme());
  for (const auto& stmt : statements_) {
    str += stmt->toString();
    str += ",";
  }
  str += ")\n";
  return str;
}

void FunctionDeclaration::accept(IASTNodeVisitor& visitor) {
  visitor.visit(*this);
}

void FunctionDeclaration::self(Program& program) {
  // main function is specific because it won't allow argument.
  // So we can use specialization to reduce instructions.
  if (name_->lexeme() == "main") {
    STARTEAR_ASSERT(args_.size() == 0);
    for (const auto& stmt : statements_) {
      static_cast<ASTNode*>(stmt.get())->self(program);
    }
  } else {
    Unimplemented();
  }
}

std::string FunctionDeclaration::toString() {
  auto func_name = fmt::format("{} (", name_->lexeme());
  for (size_t i = 0; i < args_.size(); ++i) {
    if (i == args_.size() - 1) {
      func_name += args_[i]->lexeme();
    } else {
      func_name += args_[i]->lexeme() + ", ";
    }
  }
  func_name += ") ->\n";
  for (size_t i = 0; i < statements_.size(); ++i) {
    func_name += "    ";
    func_name += statements_[i]->toString();
    if (i != statements_.size() - 1) {
      func_name += "\n";
    }
  }
  return func_name;
}

void ProgramDeclaration::accept(IASTNodeVisitor& visitor) {
  visitor.visit(*this);
}

void ProgramDeclaration::self(Program& program) {
  for (const auto& g_var : global_variable_) {
    static_cast<ASTNode*>(g_var.get())->self(program);
  }
  for (const auto& f : functions_) {
    static_cast<ASTNode*>(f.get())->self(program);
  }
  // This section is used only testing.
  for (const auto& expr : expressions_) {
    static_cast<ASTNode*>(expr.get())->self(program);
  }
}

std::string ProgramDeclaration::toString() {
  std::string program;
  for (const auto& g_var : global_variable_) {
    program += g_var->toString();
    program += "\n";
  }
  for (const auto& func : functions_) {
    program += func->toString();
    program += "\n";
  }
  // This section is used only testing.
  for (const auto& expr : expressions_) {
    program += expr->toString();
  }
  return program;
}

}  // namespace Startear