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

#ifndef STARTEAR_ALL_AST_H
#define STARTEAR_ALL_AST_H

#include <queue>
#include <variant>

#include "program.h"
#include "tokenizer.h"

namespace Startear {

class IASTNodeVisitor;

class ASTNode {
 public:
  using ASTNodePtr = std::unique_ptr<ASTNode>;

  virtual ~ASTNode() = default;

  // TODO: separate program interface and impl
  virtual void self(Program& program) = 0;

  virtual void accept(IASTNodeVisitor& visitor) = 0;

  virtual std::string toString() = 0;
};

using ASTNodePtr = ASTNode::ASTNodePtr;

class IASTNodeVisitor {
 public:
  virtual ~IASTNodeVisitor() = default;

  virtual void visit(ASTNode& node) = 0;
};

class ASTPrintVisitor : public IASTNodeVisitor {
 public:
  void visit(ASTNode& expr) override {
    std::cout << expr.toString() << std::endl;
  }
};

class StartearVMInstructionEmitter : public IASTNodeVisitor {
 public:
  void visit(ASTNode& node) override { node.self(program_); }

  const Program& emit() { return program_; }

 private:
  Program program_;
};

class BasicExpression;
using BasicExpressionPtr = std::unique_ptr<BasicExpression>;

class PrimaryExpression : public ASTNode {
 public:
  PrimaryExpression(PrimaryPtr token) : token_(std::move(token)) {}
  // Allow variable, like (+ a 3)
  PrimaryExpression(NormalPtr token) : token_(std::move(token)) {}
  PrimaryExpression(BasicExpressionPtr expr) : expr_(std::move(expr)) {}

  // ASTNode
  void accept(IASTNodeVisitor& visitor) override;
  void self(Program& program) override;
  std::string toString() override;

 private:
  TokenPtr token_;
  BasicExpressionPtr expr_;
};

using PrimaryExpressionPtr = std::unique_ptr<PrimaryExpression>;

class UnaryExpression : public ASTNode {
 public:
  using UnaryExpressionPtr = std::unique_ptr<UnaryExpression>;

  UnaryExpression(UnaryPtr token, UnaryExpressionPtr expr)
      : token_(std::move(token)), unary_expr_(std::move(expr)) {}
  UnaryExpression(PrimaryExpressionPtr expr) : primary_expr_(std::move(expr)) {}

  // ASTNode
  void accept(IASTNodeVisitor& visitor) override;
  void self(Program& program) override;
  std::string toString() override;

 private:
  TokenPtr token_;
  UnaryExpressionPtr unary_expr_;
  PrimaryExpressionPtr primary_expr_;
};

using UnaryExpressionPtr = UnaryExpression::UnaryExpressionPtr;

class MultiplicationExpression : public ASTNode {
 public:
  using MultiplicationExpressionPtr = std::unique_ptr<MultiplicationExpression>;
  MultiplicationExpression(UnaryExpressionPtr left_expr)
      : unary_left_expr_(std::move(left_expr)) {}
  MultiplicationExpression(MultiplicationPtr token,
                           UnaryExpressionPtr left_expr,
                           UnaryExpressionPtr right_expr)
      : token_(std::move(token)),
        unary_left_expr_(std::move(left_expr)),
        right_expr_(std::move(right_expr)) {}
  MultiplicationExpression(MultiplicationPtr token,
                           MultiplicationExpressionPtr left_expr,
                           UnaryExpressionPtr right_expr)
      : token_(std::move(token)),
        mul_left_expr_(std::move(left_expr)),
        right_expr_(std::move(right_expr)) {}

  // ASTNode
  void accept(IASTNodeVisitor& visitor) override;
  void self(Program& program) override;
  std::string toString() override;

 private:
  UnaryExpressionPtr unary_left_expr_;
  MultiplicationExpressionPtr mul_left_expr_;
  TokenPtr token_;
  UnaryExpressionPtr right_expr_;
};

using MultiplicationExpressionPtr =
    MultiplicationExpression::MultiplicationExpressionPtr;

class AdditionExpression : public ASTNode {
 public:
  using AdditionExpressionPtr = std::unique_ptr<AdditionExpression>;
  AdditionExpression(MultiplicationExpressionPtr left_expr)
      : mul_left_expr_(std::move(left_expr)) {}
  AdditionExpression(AdditionPtr token, MultiplicationExpressionPtr left_expr,
                     MultiplicationExpressionPtr right_expr)
      : token_(std::move(token)),
        mul_left_expr_(std::move(left_expr)),
        right_expr_(std::move(right_expr)) {}
  AdditionExpression(AdditionPtr token, AdditionExpressionPtr left_expr,
                     MultiplicationExpressionPtr right_expr)
      : token_(std::move(token)),
        add_left_expr_(std::move(left_expr)),
        right_expr_(std::move(right_expr)) {}

  // ASTNode
  void accept(IASTNodeVisitor& visitor) override;
  void self(Program& program) override;
  std::string toString() override;

 private:
  MultiplicationExpressionPtr mul_left_expr_;
  AdditionExpressionPtr add_left_expr_;
  TokenPtr token_;
  MultiplicationExpressionPtr right_expr_;
};

using AdditionExpressionPtr = AdditionExpression::AdditionExpressionPtr;

class ComparisonExpression : public ASTNode {
 public:
  using ComparisonExpressionPtr = std::unique_ptr<ComparisonExpression>;
  ComparisonExpression(AdditionExpressionPtr left_expr)
      : add_left_expr_(std::move(left_expr)) {}
  ComparisonExpression(ComparePtr token, AdditionExpressionPtr left_expr,
                       AdditionExpressionPtr right_expr)
      : token_(std::move(token)),
        add_left_expr_(std::move(left_expr)),
        right_expr_(std::move(right_expr)) {}
  ComparisonExpression(ComparePtr token, ComparisonExpressionPtr left_expr,
                       AdditionExpressionPtr right_expr)
      : token_(std::move(token)),
        cmp_left_expr_(std::move(left_expr)),
        right_expr_(std::move(right_expr)) {}

  // ASTNode
  void accept(IASTNodeVisitor& visitor) override;
  void self(Program& program) override;
  std::string toString() override;

 private:
  AdditionExpressionPtr add_left_expr_;
  ComparisonExpressionPtr cmp_left_expr_;
  TokenPtr token_;
  AdditionExpressionPtr right_expr_;
};

using ComparisonExpressionPtr = ComparisonExpression::ComparisonExpressionPtr;

class EqualityExpression : public ASTNode {
 public:
  using EqualityExpressionPtr = std::unique_ptr<EqualityExpression>;
  EqualityExpression(ComparisonExpressionPtr left_expr)
      : cmp_left_expr_(std::move(left_expr)) {}
  EqualityExpression(EqualityPtr token, ComparisonExpressionPtr left_expr,
                     ComparisonExpressionPtr right_expr)
      : token_(std::move(token)),
        cmp_left_expr_(std::move(left_expr)),
        right_expr_(std::move(right_expr)) {}
  EqualityExpression(EqualityPtr token, EqualityExpressionPtr left_expr,
                     ComparisonExpressionPtr right_expr)
      : token_(std::move(token)),
        eql_left_expr_(std::move(left_expr)),
        right_expr_(std::move(right_expr)) {}

  // ASTNode
  void accept(IASTNodeVisitor& visitor) override;
  void self(Program& program) override;
  std::string toString() override;

 private:
  ComparisonExpressionPtr cmp_left_expr_;
  EqualityExpressionPtr eql_left_expr_;
  TokenPtr token_;
  ComparisonExpressionPtr right_expr_;
};

using EqualityExpressionPtr = EqualityExpression::EqualityExpressionPtr;

class AndLogicExpression : public ASTNode {
 public:
  using AndLogicExpressionPtr = std::unique_ptr<AndLogicExpression>;
  AndLogicExpression(EqualityExpressionPtr left_expr) : eql_left_expr_(std::move(left_expr)) {}
  AndLogicExpression(EqualityPtr token, EqualityExpressionPtr left_expr,
                     EqualityExpressionPtr right_expr)
      : token_(std::move(token)),
        eql_left_expr_(std::move(left_expr)),
        eql_right_expr_(std::move(right_expr)) {}
  AndLogicExpression(EqualityPtr token, EqualityExpressionPtr left_expr,
                     AndLogicExpressionPtr right_expr) : token_(std::move(token)), eql_left_expr_(std::move(left_expr)), and_logic_right_expr_(std::move(right_expr)) {}

  // ASTNode
  void accept(IASTNodeVisitor& visitor) override;
  void self(Program& program) override;
  std::string toString() override;

 private:
  TokenPtr token_;
  EqualityExpressionPtr eql_left_expr_;
  EqualityExpressionPtr eql_right_expr_;
  AndLogicExpressionPtr and_logic_right_expr_;
};

using AndLogicExpressionPtr = AndLogicExpression::AndLogicExpressionPtr;

class OrLogicExpression : public ASTNode {
 public:
  using OrLogicExpressionPtr = std::unique_ptr<OrLogicExpression>;
  OrLogicExpression(AndLogicExpressionPtr left_expr)
      : and_logic_left_expr_(std::move(left_expr)) {}
  OrLogicExpression(EqualityPtr token, AndLogicExpressionPtr left_expr,
                    AndLogicExpressionPtr right_expr)
      : token_(std::move(token)),
        and_logic_left_expr_(std::move(left_expr)),
        and_logic_right_expr_(std::move(right_expr)) {}
  OrLogicExpression(EqualityPtr token, AndLogicExpressionPtr left_expr, OrLogicExpressionPtr right_expr)
   : token_(std::move(token)), and_logic_right_expr_(std::move(left_expr)), or_logic_expr_(std::move(right_expr)) {}

  // ASTNode
  void accept(IASTNodeVisitor& visitor) override;
  void self(Program& program) override;
  std::string toString() override;

 private:
  TokenPtr token_;
  AndLogicExpressionPtr and_logic_left_expr_;
  AndLogicExpressionPtr and_logic_right_expr_;
  OrLogicExpressionPtr or_logic_expr_;
};

using OrLogicExpressionPtr = OrLogicExpression::OrLogicExpressionPtr;

class BasicExpression : public ASTNode {
 public:
  BasicExpression(OrLogicExpressionPtr expr) : expr_(std::move(expr)) {}

  // ASTNode
  void accept(IASTNodeVisitor& visitor) override;
  void self(Program& program) override;
  std::string toString() override;

 private:
  OrLogicExpressionPtr expr_;
};

class FunctionCall : public ASTNode {
 public:
  FunctionCall(NormalPtr token, std::vector<BasicExpressionPtr>& statements)
      : token_(std::move(token)), statements_(std::move(statements)) {}

  // ASTNode
  void accept(IASTNodeVisitor& visitor) override;
  void self(Program& program) override;
  std::string toString() override;

 private:
  std::vector<BasicExpressionPtr> statements_;
  TokenPtr token_;
};

using FunctionCallPtr = std::unique_ptr<FunctionCall>;

class LetStatement : public ASTNode {
 public:
  LetStatement(NormalPtr token, BasicExpressionPtr expr)
      : token_(std::move(token)), basic_expr_(std::move(expr)) {}
  LetStatement(NormalPtr token, FunctionCallPtr expr)
      : token_(std::move(token)), func_call_(std::move(expr)) {}

  // ASTNode
  void accept(IASTNodeVisitor& visitor) override;
  void self(Program& program) override;
  std::string toString() override;

 private:
  BasicExpressionPtr basic_expr_;
  FunctionCallPtr func_call_;
  TokenPtr token_;
};

using LetStatementPtr = std::unique_ptr<LetStatement>;

class ReturnDeclaration : public ASTNode {
 public:
  ReturnDeclaration(PrimaryPtr token) : token_(std::move(token)) {}
  ReturnDeclaration(NormalPtr token) : token_(std::move(token)) {}

  // ASTNode
  void accept(IASTNodeVisitor& visitor) override;
  void self(Program& program) override;
  std::string toString() override;

 private:
  std::variant<PrimaryPtr, NormalPtr> token_;
};

using ReturnDeclarationPtr = std::unique_ptr<ReturnDeclaration>;

class FunctionDeclaration : public ASTNode {
 public:
  FunctionDeclaration(NormalPtr name_token, std::vector<TokenPtr>& args,
                      std::vector<ASTNodePtr>& statements)
      : name_(std::move(name_token)),
        args_(std::move(args)),
        statements_(std::move(statements)) {}

  // ASTNode
  void accept(IASTNodeVisitor& visitor) override;
  void self(Program& program) override;
  std::string toString() override;

 private:
  TokenPtr name_;
  std::vector<TokenPtr> args_;
  std::vector<ASTNodePtr> statements_;
};

using FunctionDeclarationPtr = std::unique_ptr<FunctionDeclaration>;

class IfStatement : public ASTNode {
 public:
  IfStatement(EqualityExpressionPtr eql_expr,
              std::vector<ASTNodePtr>& statements)
      : eql_expr_(std::move(eql_expr)), statements_(std::move(statements)) {}

  // ASTNode
  void accept(IASTNodeVisitor& visitor) override;
  void self(Program& program) override;
  std::string toString() override;

 public:
  EqualityExpressionPtr eql_expr_;
  std::vector<ASTNodePtr> statements_;
};

using IfStatementPtr = std::unique_ptr<IfStatement>;

class ProgramDeclaration : public ASTNode {
 public:
  ProgramDeclaration(std::vector<LetStatementPtr>& global_variable,
                     std::vector<FunctionDeclarationPtr>& functions,
                     std::vector<BasicExpressionPtr>&
                         expressions /* this section is used only testing */
                     )
      : global_variable_(std::move(global_variable)),
        functions_(std::move(functions)),
        expressions_(std::move(expressions)) {}

  // ASTNode
  void accept(IASTNodeVisitor& visitor) override;
  void self(Program& program) override;
  std::string toString() override;

 private:
  std::vector<LetStatementPtr> global_variable_;
  std::vector<FunctionDeclarationPtr> functions_;
  // In general, we won't accept BasicExpression on ProgramDeclaration.
  // But this class is the startup point to parse all programs.
  // So this is required when unit testing.
  std::vector<BasicExpressionPtr> expressions_;
};

using ProgramDeclarationPtr = std::unique_ptr<ProgramDeclaration>;

OPCode opcodeFromToken(TokenType token);

}  // namespace Startear

#endif  // STARTEAR_ALL_AST_H
