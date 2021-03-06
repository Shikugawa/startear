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
//        AUTHORS OR COPYRIGHT HOLDERS BE LIABcx FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "ast.h"
#include "disassembler.h"
#include "gtest/gtest.h"
#include "parser.h"
#include "program.h"
#include "startear_assert.h"
#include "tokenizer.h"
#include "vm_impl.h"

namespace Startear {
namespace {

class TokenizerTest : public testing::Test {
 public:
  void checkToken(TokenType expect_type, std::string expect_lexeme = "") {
    if (current_token_ >= result_.size()) {
      EXPECT_TRUE(false);
      return;
    }
    EXPECT_EQ(expect_type, result_[current_token_].type());

    if (expect_lexeme.size() != 0) {
      EXPECT_EQ(expect_lexeme, result_[current_token_].lexeme());
    }
    ++current_token_;
  }

  void reset() {
    result_.clear();
    current_token_ = 0;
  }

 protected:
  std::vector<Token> result_;
  size_t current_token_ = 0;
};

TEST(Basic, BasicTests) {
  Startear::Tokenizer tokenizer("+");
  auto& result = tokenizer.scanTokens();
  EXPECT_EQ(TokenType::PLUS, result[0].type());

  Startear::Tokenizer tokenizer2("-");
  result = tokenizer2.scanTokens();
  EXPECT_EQ(TokenType::MINUS, result[0].type());

  Startear::Tokenizer tokenizer3("!=");
  result = tokenizer3.scanTokens();
  EXPECT_EQ(TokenType::BANG_EQUAL, result[0].type());

  Startear::Tokenizer tokenizer4("/");
  result = tokenizer4.scanTokens();
  EXPECT_EQ(TokenType::SLASH, result[0].type());

  Startear::Tokenizer tokenizer5("// test\n");
  result = tokenizer5.scanTokens();
  EXPECT_EQ(TokenType::COMMENT, result[0].type());

  Startear::Tokenizer tokenizer6(" test");
  result = tokenizer6.scanTokens();
  EXPECT_EQ("test", result[0].lexeme());

  Startear::Tokenizer tokenizer7("\"sample\"");
  result = tokenizer7.scanTokens();
  EXPECT_EQ(TokenType::STRING, result[0].type());

  Startear::Tokenizer tokenizer8("sample");
  result = tokenizer8.scanTokens();
  EXPECT_EQ("sample", result[0].lexeme());

  Startear::Tokenizer tokenizer9("\"sample");
  result = tokenizer9.scanTokens();
  EXPECT_EQ(result.size(), 0);

  Startear::Tokenizer tokenizer10("123;");
  result = tokenizer10.scanTokens();
  EXPECT_EQ(TokenType::NUMBER, result[0].type());
  EXPECT_EQ("123", result[0].lexeme());
  EXPECT_EQ(TokenType::SEMICOLON, result[1].type());

  Startear::Tokenizer tokenizer11("let vault");
  result = tokenizer11.scanTokens();
  EXPECT_EQ(TokenType::VAR, result[0].type());
  EXPECT_EQ("vault", result[1].lexeme());
  EXPECT_EQ(TokenType::IDENTIFIER, result[1].type());
}

TEST_F(TokenizerTest, ForTest) {
  Startear::Tokenizer tokenizer("for (let i = 0.0000; i < 65535; i++) {}");
  result_ = tokenizer.scanTokens();
  checkToken(TokenType::FOR);
  checkToken(TokenType::LEFT_PAREN);
  checkToken(TokenType::VAR);
  checkToken(TokenType::IDENTIFIER, "i");
  checkToken(TokenType::EQUAL);
  checkToken(TokenType::NUMBER, "0.0000");
  checkToken(TokenType::SEMICOLON);
  checkToken(TokenType::IDENTIFIER, "i");
  checkToken(TokenType::LESS);
  checkToken(TokenType::NUMBER, "65535");
  checkToken(TokenType::SEMICOLON);
  checkToken(TokenType::IDENTIFIER, "i");
  checkToken(TokenType::PLUS);
  checkToken(TokenType::PLUS);
  checkToken(TokenType::RIGHT_PAREN);
  checkToken(TokenType::LEFT_BRACE);
  checkToken(TokenType::RIGHT_BRACE);
  reset();
}

TEST_F(TokenizerTest, IfTest) {
  Startear::Tokenizer tokenizer("if (args == 0) {}");
  result_ = tokenizer.scanTokens();
  checkToken(TokenType::IF);
  checkToken(TokenType::LEFT_PAREN);
  checkToken(TokenType::IDENTIFIER);
  checkToken(TokenType::EQUAL_EQUAL);
  checkToken(TokenType::NUMBER, "0");
  checkToken(TokenType::RIGHT_PAREN);
  checkToken(TokenType::LEFT_BRACE);
  checkToken(TokenType::RIGHT_BRACE);
}

TEST_F(TokenizerTest, CodeBlockTest) {
  std::string code = R"(
fn main(arg1, arg2) {
    let a = 32;
    let b = 43;
    return a + b;
}
                               )";
  Startear::Tokenizer tokenizer(code);
  result_ = tokenizer.scanTokens();
  checkToken(TokenType::FUN);
  checkToken(TokenType::IDENTIFIER, "main");
  checkToken(TokenType::LEFT_PAREN);
  checkToken(TokenType::IDENTIFIER, "arg1");
  checkToken(TokenType::COMMA);
  checkToken(TokenType::IDENTIFIER, "arg2");
  checkToken(TokenType::RIGHT_PAREN);
  checkToken(TokenType::LEFT_BRACE);
  checkToken(TokenType::VAR);
  checkToken(TokenType::IDENTIFIER, "a");
  checkToken(TokenType::EQUAL);
  checkToken(TokenType::NUMBER, "32");
  checkToken(TokenType::SEMICOLON);
  checkToken(TokenType::VAR);
  checkToken(TokenType::IDENTIFIER, "b");
  checkToken(TokenType::EQUAL);
  checkToken(TokenType::NUMBER, "43");
  checkToken(TokenType::SEMICOLON);
  checkToken(TokenType::RETURN);
  checkToken(TokenType::IDENTIFIER, "a");
  checkToken(TokenType::PLUS);
  checkToken(TokenType::IDENTIFIER, "b");
  checkToken(TokenType::SEMICOLON);
  reset();
}

class ParserTest : public testing::Test {
 public:
  void run(std::string code, std::string expected) {
    testing::internal::CaptureStdout();
    auto tokenizer = Tokenizer(code);
    auto tokens = tokenizer.scanTokens();
    auto p = Parser(tokens);
    auto result = p.parse();
    ASTPrintVisitor visitor;
    result->accept(visitor);
    EXPECT_EQ(expected, testing::internal::GetCapturedStdout());
  }
};

TEST_F(ParserTest, BasicTest) {
  run("2 + 3", "(+ 2 3)\n");
  run("32 + 21 / 21", "(+ 32 (/ 21 21))\n");
  run("(32 + 21) / 21", "(/ (+ 32 21) 21)\n");
  run("(32 / (32 + 32)) / 32", "(/ (/ 32 (+ 32 32)) 32)\n");
  run("0 == 3", "(== 0 3)\n");
  run("0 >= 3", "(>= 0 3)\n");
  run("0 <= 3", "(<= 0 3)\n");
  run("0 != 3", "(!= 0 3)\n");
  run("0 < 3", "(< 0 3)\n");
  run("0 > 3", "(> 0 3)\n");
  run("0 == (3 == 4)", "(== 0 (== 3 4))\n");
  run("2 == 2 || 2 == 3", "(|| (== 2 2) (== 2 3))\n");
}

TEST_F(ParserTest, FuncTest) {
  std::string code = R"(
fn main(arg1, arg2) {
    let a = 3;
    let b = 4;
}

fn main2(arg) {}
)";
  std::string expected = R"(main (arg1, arg2) ->
    a -> 3
    b -> 4
main2 (arg) ->


)";
  run(code, expected);
}

class EmitterTest : public testing::Test {
 public:
  void run(std::string code) {
    auto tokenizer = Tokenizer(code);
    auto tokens = tokenizer.scanTokens();
    auto p = Parser(tokens);
    auto result = p.parse();
    ASTPrintVisitor v;
    result->accept(v);
    result->accept(emitter_);
  }

  StartearVMInstructionEmitter emitter_;
};

TEST_F(EmitterTest, BasicTest) {
  run("2 + 3");
  auto program = emitter_.emit();
  ASSERT_EQ(program.fetchInst(0)->get().opcode(), OPCode::OP_PUSH);
  ASSERT_EQ(program.fetchInst(0)->get().operandsPointer()[0], 0);
  ASSERT_EQ(program.fetchInst(1)->get().opcode(), OPCode::OP_PUSH);
  ASSERT_EQ(program.fetchInst(1)->get().operandsPointer()[0], 1);
  ASSERT_EQ(program.fetchInst(2)->get().opcode(), OPCode::OP_ADD);
  ASSERT_EQ(program.fetchValue(0)->getDouble(), 2.0);
  ASSERT_EQ(program.fetchValue(1)->getDouble(), 3.0);
}

TEST_F(EmitterTest, StoreVariable) {
  run("let a = 3 + 1;");
  auto program = emitter_.emit();
  ASSERT_EQ(program.fetchInst(0)->get().opcode(), OPCode::OP_PUSH);
  ASSERT_EQ(program.fetchInst(0)->get().operandsPointer()[0], 0);
  ASSERT_EQ(program.fetchInst(1)->get().opcode(), OPCode::OP_PUSH);
  ASSERT_EQ(program.fetchInst(1)->get().operandsPointer()[0], 1);
  ASSERT_EQ(program.fetchInst(2)->get().opcode(), OPCode::OP_ADD);
  ASSERT_EQ(program.fetchInst(3)->get().opcode(), OPCode::OP_STORE_LOCAL);
  disassemble(program);
}

TEST_F(EmitterTest, StoreVariable2) {
  run("let b = a + 2;");
  auto program = emitter_.emit();
  ASSERT_EQ(program.fetchInst(0)->get().opcode(), OPCode::OP_LOAD_LOCAL);
  ASSERT_EQ(program.fetchInst(0)->get().operandsPointer()[0], 0);
  ASSERT_EQ(program.fetchInst(1)->get().opcode(), OPCode::OP_PUSH);
  ASSERT_EQ(program.fetchInst(1)->get().operandsPointer()[0], 1);
  ASSERT_EQ(program.fetchInst(2)->get().opcode(), OPCode::OP_ADD);
  ASSERT_EQ(program.fetchInst(3)->get().opcode(), OPCode::OP_STORE_LOCAL);
  disassemble(program);
}

TEST_F(EmitterTest, Compare) {
  run("3 == 2");
  auto program = emitter_.emit();
  ASSERT_EQ(program.fetchInst(0)->get().opcode(), OPCode::OP_PUSH);
  ASSERT_EQ(program.fetchInst(0)->get().operandsPointer()[0], 0);
  ASSERT_EQ(program.fetchInst(1)->get().opcode(), OPCode::OP_PUSH);
  ASSERT_EQ(program.fetchInst(1)->get().operandsPointer()[0], 1);
  ASSERT_EQ(program.fetchInst(2)->get().opcode(), OPCode::OP_EQUAL);
  disassemble(program);
}

TEST_F(EmitterTest, Compare2) {
  run("3 != 2");
  auto program2 = emitter_.emit();
  ASSERT_EQ(program2.fetchInst(0)->get().opcode(), OPCode::OP_PUSH);
  ASSERT_EQ(program2.fetchInst(0)->get().operandsPointer()[0], 0);
  ASSERT_EQ(program2.fetchInst(1)->get().opcode(), OPCode::OP_PUSH);
  ASSERT_EQ(program2.fetchInst(1)->get().operandsPointer()[0], 1);
  ASSERT_EQ(program2.fetchInst(2)->get().opcode(), OPCode::OP_BANG_EQUAL);
  disassemble(program2);
}

TEST_F(EmitterTest, Compare3) {
  run("3 > 2");
  auto program2 = emitter_.emit();
  ASSERT_EQ(program2.fetchInst(0)->get().opcode(), OPCode::OP_PUSH);
  ASSERT_EQ(program2.fetchInst(0)->get().operandsPointer()[0], 0);
  ASSERT_EQ(program2.fetchInst(1)->get().opcode(), OPCode::OP_PUSH);
  ASSERT_EQ(program2.fetchInst(1)->get().operandsPointer()[0], 1);
  ASSERT_EQ(program2.fetchInst(2)->get().opcode(), OPCode::OP_GREATER);
  disassemble(program2);
}

TEST_F(EmitterTest, Compare4) {
  run("3 >= 2");
  auto program2 = emitter_.emit();
  ASSERT_EQ(program2.fetchInst(0)->get().opcode(), OPCode::OP_PUSH);
  ASSERT_EQ(program2.fetchInst(0)->get().operandsPointer()[0], 0);
  ASSERT_EQ(program2.fetchInst(1)->get().opcode(), OPCode::OP_PUSH);
  ASSERT_EQ(program2.fetchInst(1)->get().operandsPointer()[0], 1);
  ASSERT_EQ(program2.fetchInst(2)->get().opcode(), OPCode::OP_GREATER_EQUAL);
  disassemble(program2);
}

TEST_F(EmitterTest, Compare5) {
  run("3 < 2");
  auto program2 = emitter_.emit();
  ASSERT_EQ(program2.fetchInst(0)->get().opcode(), OPCode::OP_PUSH);
  ASSERT_EQ(program2.fetchInst(0)->get().operandsPointer()[0], 0);
  ASSERT_EQ(program2.fetchInst(1)->get().opcode(), OPCode::OP_PUSH);
  ASSERT_EQ(program2.fetchInst(1)->get().operandsPointer()[0], 1);
  ASSERT_EQ(program2.fetchInst(2)->get().opcode(), OPCode::OP_LESS);
  disassemble(program2);
}

TEST_F(EmitterTest, Compare6) {
  run("3 <= 2 || 3 == 3");
  auto program2 = emitter_.emit();
  ASSERT_EQ(program2.fetchInst(0)->get().opcode(), OPCode::OP_PUSH);
  ASSERT_EQ(program2.fetchInst(0)->get().operandsPointer()[0], 0);
  ASSERT_EQ(program2.fetchInst(1)->get().opcode(), OPCode::OP_PUSH);
  ASSERT_EQ(program2.fetchInst(1)->get().operandsPointer()[0], 1);
  ASSERT_EQ(program2.fetchInst(2)->get().opcode(), OPCode::OP_LESS_EQUAL);
  disassemble(program2);
}

class VMExecIntegration : public testing::Test {
 public:
  void prepare(std::string& code, std::function<void(Program&)> program_eval,
               std::function<void(VMImpl&)> vm_eval, bool dbg) {
    Tokenizer t(code);
    Parser p(t.scanTokens());

    auto ast = p.parse();
    StartearVMInstructionEmitter emitter;
    ast->accept(emitter);

    auto program = emitter.emit();
    program_eval(program);

    VMImpl vm(program);

    if (dbg) {
      ASTPrintVisitor v;
      ast->accept(v);
      disassemble(program);
    }

    vm.start();
    vm_eval(vm);
  }
};

TEST_F(VMExecIntegration, NoStatement) {
  std::string code = R"(
fn main() {}
)";
  prepare(
      code,
      [&](Program& program) {
        EXPECT_EQ(program.instructions()[0].opcode(), OPCode::OP_RETURN);
      },
      [&](VMImpl& vm) {}, true);
}

TEST_F(VMExecIntegration, Cond) {
  std::string code = R"(
fn main() {
  let p = 3 > 2;
  let q = 3 < 2;
  let r = 3 == 3;
  let s = 3 != 3;
  let u = 3 == 2;
  let t = 3 == 3 || 3 == 2;
}
)";
  prepare(
      code, [&](Program& program) {},
      [&](VMImpl& vm) {
        const auto& top_frame = vm.peekFrame();
        const auto& entry_p = top_frame.lv_table_.find("p");
        ASSERT_TRUE(entry_p != top_frame.lv_table_.end());
        ASSERT_EQ(entry_p->second.getDouble().value(), 1.0);
        const auto& entry_q = top_frame.lv_table_.find("q");
        ASSERT_TRUE(entry_q != top_frame.lv_table_.end());
        ASSERT_EQ(entry_q->second.getDouble().value(), 0.0);
        const auto& entry_r = top_frame.lv_table_.find("r");
        ASSERT_TRUE(entry_r != top_frame.lv_table_.end());
        ASSERT_EQ(entry_r->second.getDouble().value(), 1.0);
        const auto& entry_s = top_frame.lv_table_.find("s");
        ASSERT_TRUE(entry_s != top_frame.lv_table_.end());
        ASSERT_EQ(entry_s->second.getDouble().value(), 0.0);
        const auto& entry_u = top_frame.lv_table_.find("u");
        ASSERT_TRUE(entry_u != top_frame.lv_table_.end());
        ASSERT_EQ(entry_u->second.getDouble().value(), 0.0);
        const auto& entry_t = top_frame.lv_table_.find("t");
        ASSERT_TRUE(entry_t != top_frame.lv_table_.end());
        ASSERT_EQ(entry_t->second.getDouble().value(), 1.0);
      },
      true);
}

TEST_F(VMExecIntegration, BasicCalc) {
  std::string code = R"(
fn main() {
    // test comment
    let a = 3;
    let b = 4;
    let c = a + b;
}
)";
  prepare(
      code,
      [&](Program& program) {
        EXPECT_EQ(program.instructions()[0].opcode(), OPCode::OP_PUSH);
        EXPECT_EQ(program.instructions()[1].opcode(), OPCode::OP_STORE_LOCAL);
        EXPECT_EQ(program.instructions()[2].opcode(), OPCode::OP_PUSH);
        EXPECT_EQ(program.instructions()[3].opcode(), OPCode::OP_STORE_LOCAL);
        EXPECT_EQ(program.instructions()[4].opcode(), OPCode::OP_LOAD_LOCAL);
        EXPECT_EQ(program.instructions()[5].opcode(), OPCode::OP_LOAD_LOCAL);
        EXPECT_EQ(program.instructions()[6].opcode(), OPCode::OP_ADD);
        EXPECT_EQ(program.instructions()[7].opcode(), OPCode::OP_STORE_LOCAL);
      },
      [&](VMImpl& vm) {
        const auto& top_frame = vm.peekFrame();
        // Variable State
        ASSERT_EQ(top_frame.lv_table_.size(), 3);

        const auto& entry_a = top_frame.lv_table_.find("a");
        ASSERT_TRUE(entry_a != top_frame.lv_table_.end());
        ASSERT_EQ(entry_a->second.getDouble().value(), 3.0);

        const auto& entry_b = top_frame.lv_table_.find("b");
        ASSERT_TRUE(entry_b != top_frame.lv_table_.end());
        ASSERT_EQ(entry_b->second.getDouble().value(), 4.0);

        const auto& entry_c = top_frame.lv_table_.find("c");
        ASSERT_TRUE(entry_c != top_frame.lv_table_.end());
        ASSERT_EQ(entry_c->second.getDouble().value(), 7.0);
      },
      true);
}

TEST_F(VMExecIntegration, Substitution) {
  std::string code = R"(
fn main() {
    let a = 3;
    a = 4;
}
)";
  prepare(
      code,
      [&](Program& program) {
        EXPECT_EQ(program.instructions()[0].opcode(), OPCode::OP_PUSH);
        EXPECT_EQ(program.instructions()[1].opcode(), OPCode::OP_STORE_LOCAL);
        EXPECT_EQ(program.instructions()[2].opcode(), OPCode::OP_PUSH);
        EXPECT_EQ(program.instructions()[3].opcode(), OPCode::OP_STORE_LOCAL);
      },
      [&](VMImpl& vm) {
        const auto& top_frame = vm.peekFrame();
        // Variable State
        ASSERT_EQ(top_frame.lv_table_.size(), 1);

        const auto& entry_a = top_frame.lv_table_.find("a");
        ASSERT_TRUE(entry_a != top_frame.lv_table_.end());
        ASSERT_EQ(entry_a->second.getDouble().value(), 4.0);
      },
      true);
}

TEST_F(VMExecIntegration, IfAndSubstitution) {
  std::string code = R"(
fn main() {
    let a = 3;
    if (a != 3) {
      a = 2;
    }
}
)";
  prepare(
      code,
      [&](Program& program) {
        EXPECT_EQ(program.instructions()[0].opcode(), OPCode::OP_PUSH);
        EXPECT_EQ(program.instructions()[1].opcode(), OPCode::OP_STORE_LOCAL);
        EXPECT_EQ(program.instructions()[2].opcode(), OPCode::OP_LOAD_LOCAL);
        EXPECT_EQ(program.instructions()[3].opcode(), OPCode::OP_PUSH);
        EXPECT_EQ(program.instructions()[4].opcode(), OPCode::OP_BANG_EQUAL);
        EXPECT_EQ(program.instructions()[5].opcode(), OPCode::OP_BRANCH);
        EXPECT_EQ(program.instructions()[6].opcode(), OPCode::OP_PUSH);
        EXPECT_EQ(program.instructions()[7].opcode(), OPCode::OP_STORE_LOCAL);
      },
      [&](VMImpl& vm) {
        const auto& top_frame = vm.peekFrame();
        // Variable State
        ASSERT_EQ(top_frame.lv_table_.size(), 1);

        const auto& entry_a = top_frame.lv_table_.find("a");
        ASSERT_TRUE(entry_a != top_frame.lv_table_.end());
        ASSERT_EQ(entry_a->second.getDouble().value(), 3.0);
      },
      true);
}

TEST_F(VMExecIntegration, FuncCall) {
  std::string code = R"(
fn sub(arg1, arg2) {
    let q = arg1 + arg2;
    return q;
}

fn main() {
    let b = sub(9, 10);
}
)";
  prepare(
      code,
      [&](Program& program) {
        EXPECT_EQ(program.instructions()[0].opcode(), OPCode::OP_LOAD_LOCAL);
        EXPECT_EQ(program.instructions()[1].opcode(), OPCode::OP_LOAD_LOCAL);
        // start function call
        EXPECT_EQ(program.instructions()[2].opcode(), OPCode::OP_ADD);
        EXPECT_EQ(program.instructions()[3].opcode(), OPCode::OP_STORE_LOCAL);
        EXPECT_EQ(program.instructions()[4].opcode(), OPCode::OP_LOAD_LOCAL);
        EXPECT_EQ(program.instructions()[5].opcode(), OPCode::OP_RETURN);
        EXPECT_EQ(program.instructions()[6].opcode(), OPCode::OP_PUSH);
        EXPECT_EQ(program.instructions()[7].opcode(), OPCode::OP_PUSH);
        EXPECT_EQ(program.instructions()[8].opcode(), OPCode::OP_CALL);
        EXPECT_EQ(program.instructions()[9].opcode(), OPCode::OP_STORE_LOCAL);
      },
      [&](VMImpl& vm) {
        const auto& top_frame = vm.peekFrame();

        // Variable state
        ASSERT_EQ(top_frame.lv_table_.size(), 1);
        const auto& entry_b = top_frame.lv_table_.find("b");
        ASSERT_TRUE(entry_b != top_frame.lv_table_.end());
        ASSERT_EQ(entry_b->second.getDouble().value(), 19.0);
      },
      true);
}

TEST_F(VMExecIntegration, NestReturn) {
  std::string code = R"(
fn calc(num) {
  if (num == 0) {
    return 1;
  }
  return 2;
}

fn main() {
  let a = calc(0);
}
)";
  prepare(
      code, [&](Program& program) {},
      [&](VMImpl& vm) {
        const auto& top_frame = vm.peekFrame();

        // Variable state
        const auto& entry_a = top_frame.lv_table_.find("a");
        ASSERT_TRUE(entry_a != top_frame.lv_table_.end());
        ASSERT_EQ(entry_a->second.getDouble().value(), 1.0);
      },
      true);
}

TEST_F(VMExecIntegration, Fibonacchi) {
  std::string code = R"(
fn calc(num) {
  if (num == 0 || num == 1) {
    return 1;
  }
  let acc = calc(num) + calc(num-1);
  return acc;
}

fn main() {
  let a = calc(3);
}
)";
  prepare(
      code, [&](Program& program) {},
      [&](VMImpl& vm) {
        const auto& top_frame = vm.peekFrame();

        // Variable state
        const auto& entry_a = top_frame.lv_table_.find("a");
        ASSERT_TRUE(entry_a != top_frame.lv_table_.end());
        ASSERT_EQ(entry_a->second.getDouble().value(), 2.0);
      },
      true);
}

}  // namespace
}  // namespace Startear
