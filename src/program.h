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

#ifndef STARTEAR_ALL_PROGRAM_H
#define STARTEAR_ALL_PROGRAM_H

#include <cstddef>
#include <cstring>
#include <deque>
#include <iostream>
#include <memory>
#include <optional>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "assert.h"
#include "opcode.h"

namespace Startear {

class Value {
 public:
  enum SupportedTypes {
    // Value is not set.
    None,
    // Used to specify variable name in the instruction sequence, or treat
    // string directly.
    String,
    // Normally, all of number is treated as double in the world of instruction
    // sequence.
    Double
  };

  enum Category { Variable, Literal };

  template <typename T>
  Value(Category c, T v);
  Value(Category c) : category_(c) {}

  std::optional<const char*> getString() const;
  std::optional<double> getDouble() const;

  Category category() { return category_; }
  SupportedTypes type() { return type_; }

 private:
  void setString(const char* s, size_t len);
  void setDouble(double d);

 protected:
  std::byte* bytes_;
  SupportedTypes type_{SupportedTypes::None};
  Category category_;
};

template <typename T>
Value::Value(Category c, T v) : category_(c) {
  if constexpr (std::is_same<std::decay_t<decltype(v)>, std::string>::value) {
    type_ = SupportedTypes::String;
    setString(v.c_str(), v.size());
  } else if (std::is_same<std::decay_t<decltype(v)>, double>::value) {
    type_ = SupportedTypes::Double;
    setDouble(v);
  } else {
    NOT_REACHED;
  }
}

class Program {
 public:
  // Instructions
  template <typename T>
  void addInst(OPCode code,
               std::initializer_list<std::pair<Value::Category, T>> operands);
  void addInst(OPCode code);
  std::optional<std::pair<OPCode, std::deque<size_t>>> fetchInst();

  // Value
  size_t addValue(Value v);
  std::optional<Value> fetchValue(size_t i);

  void updateIndex(size_t i);

  struct FunctionMetadata {
    size_t pc_;    // Program counter of specified function.
    size_t args_;  // The number of arguments.
  };

  // Function symbol tables.
  // Register symbol name and current top instruction pointer.
  // This function is used if you'd like to create function from bytecode
  // generation AST visitor.
  void addFunction(std::string name, size_t args);
  std::optional<FunctionMetadata> getFunction(std::string name);

  // Properties
  const std::vector<size_t>& instructions() { return instructions_; }
  const std::vector<Value>& values() { return values_; }

 private:
  std::optional<std::pair<OPCode, uint8_t>> consume();
  bool validOperandSize(OPCode code, size_t operand_size);
  bool isProgramEnd() const { return index_ >= instructions_.size(); }

  std::vector<size_t> instructions_;
  std::vector<Value> values_;
  // This is a pair of function label and pointer in the instructions.
  // For example, we are provided these instructions,
  //
  // 16 | OP_PUSH 32 <- function sample
  // 17 | OP_PUSH 35
  // 18 | OP_ADD
  // 19 | OP_RETURN
  //
  // In this case, we set the pair {"sample", 16} in this hash table.
  std::unordered_map<std::string, FunctionMetadata> func_metadata_;
  // This value indicates pure index of instructions.
  // The difference between index and program counter is,
  // program counter don't care about operands.
  // e.g.
  // OP_PUSH 3 OP_PUSH 4 OP_ADD
  //    ↑    ↑    ↑
  //    pc index pc(next)
  size_t index_{0};
};

template <typename T>
void Program::addInst(
    OPCode code,
    std::initializer_list<std::pair<Value::Category, T>> operands) {
  addInst(code);
  if (!validOperandSize(code, operands.size())) {
    return;
  }
  for (const auto& [category, operand] : operands) {
    Value v(category, operand);
    auto ptr = addValue(v);
    instructions_.emplace_back(ptr);
  }
}

}  // namespace Startear

#endif  // STARTEAR_ALL_PROGRAM_H
