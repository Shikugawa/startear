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

#include "opcode.h"
#include "startear_assert.h"

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

class Instruction {
 public:
  Instruction(OPCode code) : code_(code) {}
  template <class It>
  Instruction(OPCode code, It begin, It end)
      : code_(code), operands_ptr_(begin, end) {}

  const OPCode opcode() const { return code_; }
  const std::vector<size_t>& operandsPointer() const { return operands_ptr_; }

 private:
  OPCode code_;
  std::vector<size_t> operands_ptr_;
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
  std::optional<std::reference_wrapper<const Instruction>> fetchInst(size_t pc);

  // Value
  size_t addValue(Value v);
  std::optional<Value> fetchValue(size_t i);

  struct FunctionMetadata {
    std::string name_;
    size_t pc_;  // Program counter of specified function.
    std::vector<size_t>
        args_;  // The pointers to argument names for temporal use.
  };

  struct FunctionRegistry {
    std::optional<std::reference_wrapper<const FunctionMetadata>>
    findByProgramCounter(size_t line) const;
    std::optional<std::reference_wrapper<const FunctionMetadata>> findByName(
        std::string name) const;
    void registerFunction(std::string name, std::vector<size_t>& args,
                          size_t pc);

   private:
    // TODO: replace flat hash map
    std::unordered_map<size_t, std::string> pc_name_;
    std::unordered_map<std::string, FunctionMetadata> metadata_;
  };

  friend FunctionRegistry;

  // Function symbol tables.
  // Register symbol name and current top instruction pointer.
  // This function is used if you'd like to create function from bytecode
  // generation AST visitor.
  void addFunction(std::string name, std::vector<size_t>& args);
  const FunctionRegistry& functionRegistry() const {
    return registered_function_;
  }

  // Properties
  const std::vector<Instruction>& instructions() { return instructions_; }
  const std::vector<Value>& values() { return values_; }

 private:
  bool isProgramEnd(size_t pc) const { return pc >= instructions_.size(); }

  std::vector<Instruction> instructions_;
  std::vector<Value> values_;
  // This is a pair of function label and pointer in the instructions.
  // For example, try to consider this function
  //
  // fn sample() {
  //    let a = 32;
  //    let b = 35;
  //    return a + b;
  // }
  //
  // in this case, we can get these instructions,
  //
  // 16 | OP_PUSH 32 <- function sample
  // 17 | OP_PUSH 35
  // 18 | OP_ADD
  // 19 | OP_RETURN
  //
  // In this case, we set the pair {"sample", {16, 0}} in this hash table.
  FunctionRegistry registered_function_;
};

template <typename T>
void Program::addInst(
    OPCode code,
    std::initializer_list<std::pair<Value::Category, T>> operands) {
  if (!validOperandSize(code, operands.size())) {
    return;
  }
  std::vector<size_t> operands_ptr;
  for (const auto& [category, operand] : operands) {
    Value v(category, operand);
    auto ptr = addValue(v);
    operands_ptr.emplace_back(ptr);
  }
  instructions_.emplace_back(code, operands_ptr.begin(), operands_ptr.end());
}

}  // namespace Startear

#endif  // STARTEAR_ALL_PROGRAM_H
