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
          OPCode::OP_LOAD_LOCAL,
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
                       add_left_expr_->toString(), right_expr_->toString());
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
  if (token_ != nullptr) {
    program.addInst(opcodeFromToken(token_->type()));
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
  if (token_ != nullptr) {
    program.addInst(opcodeFromToken(token_->type()));
  }
}

std::string BasicExpression::toString() { return expr_->toString(); }

void BasicExpression::accept(IASTNodeVisitor& visitor) { visitor.visit(*this); }

void BasicExpression::self(Program& program) {
  static_cast<ASTNode*>(expr_.get())->self(program);
}

std::string LetStatement::toString() {
  if (basic_expr_ != nullptr) {
    return fmt::format("{} -> {}", token_->lexeme(), basic_expr_->toString());
  } else if (func_call_ != nullptr) {
    return fmt::format("{} -> {}", token_->lexeme(), func_call_->toString());
  }
  NOT_REACHED;
}

void LetStatement::accept(IASTNodeVisitor& visitor) { visitor.visit(*this); }

void LetStatement::self(Program& program) {
  if (basic_expr_ != nullptr) {
    static_cast<ASTNode*>(basic_expr_.get())->self(program);
  } else if (func_call_ != nullptr) {
    static_cast<ASTNode*>(func_call_.get())->self(program);
  }
  program.addInst(OPCode::OP_STORE_LOCAL,
                  {std::make_pair(Value::Category::Literal, token_->lexeme())});
}

void FunctionCall::accept(IASTNodeVisitor& visitor) { visitor.visit(*this); }

void FunctionCall::self(Program& program) {
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
  std::vector<size_t> argname_ptrs;
  for (const auto& arg : args_) {
    Value v(Value::Category::Variable, arg->lexeme());
    const auto v_ptr = program.addValue(v);
    argname_ptrs.emplace_back(v_ptr);
  }
  program.addFunction(name_->lexeme(), argname_ptrs);
  // Just to add return instruction when there is no statement in this function.
  if (statements_.size() == 0) {
    program.addInst(OPCode::OP_RETURN);
    return;
  }
  for (const auto& stmt : statements_) {
    static_cast<ASTNode*>(stmt.get())->self(program);
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

void ReturnDeclaration::accept(IASTNodeVisitor& visitor) {
  visitor.visit(*this);
}

void ReturnDeclaration::self(Program& program) {
  if (std::holds_alternative<PrimaryPtr>(token_)) {  // Number
    program.addInst(
        OPCode::OP_PUSH,
        {std::make_pair(Value::Category::Literal,
                        std::stod(std::get<PrimaryPtr>(token_)->lexeme()))});
  } else if (std::holds_alternative<NormalPtr>(token_)) {  // Identifier
    program.addInst(OPCode::OP_LOAD_LOCAL,
                    {std::make_pair(Value::Category::Variable,
                                    std::get<NormalPtr>(token_)->lexeme())});
  }
  program.addInst(OPCode::OP_RETURN);
}

std::string ReturnDeclaration::toString() {
  return std::visit(
      [](auto& token) -> std::string {
        return fmt::format("return {}", token->lexeme());
      },
      token_);
}

void IfStatement::accept(IASTNodeVisitor& visitor) { visitor.visit(*this); }

void IfStatement::self(Program& program) {
  static_cast<ASTNode*>(eql_expr_.get())->self(program);
  std::string label_if_entry = program.getIndexedLabel();
  std::string label_not_if_entry = program.getIndexedLabel();
  program.addInst(OPCode::OP_BRANCH, {
    std::make_pair(Value::Category::Literal, label_if_entry),
    std::make_pair(Value::Category::Literal, label_not_if_entry)});
  for (auto i = 0; i < statements_.size(); ++i) {
    if (i == 0) {
      program.addLabel(label_if_entry);
    }
    static_cast<ASTNode*>(statements_[i].get())->self(program);
  }
  program.addLabel(label_not_if_entry);
}

std::string IfStatement::toString() {
  std::string program;
  program += fmt::format("if ({})\n", eql_expr_->toString());
  for (const auto& stmt : statements_) {
    program += fmt::format("\t{}\n", stmt->toString());
  }
  return program;
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

OPCode opcodeFromToken(TokenType token) {
  switch (token) {
    case TokenType::EQUAL_EQUAL:
      return OPCode::OP_EQUAL;
    case TokenType::BANG_EQUAL:
      return OPCode::OP_BANG_EQUAL;
    case TokenType::LESS_EQUAL:
      return OPCode::OP_LESS_EQUAL;
    case TokenType::GREATER_EQUAL:
      return OPCode::OP_GREATER_EQUAL;
    case TokenType::LESS:
      return OPCode::OP_LESS;
    case TokenType::GREATER:
      return OPCode::OP_GREATER;
    default:
      NOT_REACHED;
  }
}

}  // namespace Startear