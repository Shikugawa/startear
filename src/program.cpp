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

#include "program.h"

#include <cstring>

#include "startear_assert.h"

namespace Startear {

void Value::setString(const char* s, size_t len) {
  bytes_ = reinterpret_cast<std::byte*>(calloc(len, sizeof(char)));
  memcpy(bytes_, s, len * sizeof(char));
}

void Value::setDouble(double d) {
  type_ = SupportedTypes::Double;
  bytes_ = reinterpret_cast<std::byte*>(calloc(1, sizeof(double)));
  memcpy(bytes_, &d, sizeof(double));
}

std::optional<const char*> Value::getString() const {
  if (type_ != SupportedTypes::String) {
    return std::nullopt;
  }
  auto* v_ptr = reinterpret_cast<const char*>(bytes_);
  return v_ptr;
}

std::optional<double> Value::getDouble() const {
  STARTEAR_ASSERT(category_ == Value::Literal);
  if (type_ != SupportedTypes::Double) {
    return std::nullopt;
  }
  auto* v_ptr = reinterpret_cast<double*>(bytes_);
  return *v_ptr;
}

void Program::addInst(OPCode code) { instructions_.emplace_back(code); }

std::optional<std::reference_wrapper<const Instruction>> Program::fetchInst(
    size_t pc) {
  if (isProgramEnd(pc)) {
    return std::nullopt;
  }
  return instructions_[pc];
}

std::optional<Value> Program::fetchValue(size_t i) {
  if (i >= values_.size()) {
    return std::nullopt;
  }
  return values_.at(i);
}

size_t Program::addValue(Value v) {
  values_.emplace_back(v);
  return values_.size() - 1;
}

void Program::addFunction(std::string name, std::vector<size_t>& args) {
  auto current_top = instructions_.size();
  registered_function_.registerFunction(name, args, current_top);
}

std::optional<std::reference_wrapper<const Program::FunctionMetadata>>
Program::FunctionRegistry::findByProgramCounter(size_t line) const {
  auto itr = pc_name_.find(line);
  if (itr == pc_name_.end()) {
    return std::nullopt;
  }
  return findByName(itr->second);
}

std::optional<std::reference_wrapper<const Program::FunctionMetadata>>
Program::FunctionRegistry::findByName(std::string name) const {
  auto itr2 = metadata_.find(name);
  if (itr2 == metadata_.end()) {
    return std::nullopt;
  }
  return std::reference_wrapper(itr2->second);
}

void Program::FunctionRegistry::registerFunction(std::string name, std::vector<size_t>& args,
                                                 size_t pc) {
  pc_name_.emplace(std::make_pair(pc, name));
  metadata_.emplace(
      std::make_pair(name, Program::FunctionMetadata{name, pc, args}));
  STARTEAR_ASSERT(pc_name_.size() == metadata_.size());
}

bool Program::validOperandSize(OPCode code, size_t operand_size) {
  const auto expect_size = [&operand_size](size_t expected) {
    return operand_size == expected;
  };
  switch (code) {
    case OPCode::OP_PRINT:
    case OPCode::OP_PUSH:
    case OPCode::OP_STORE_LOCAL:
    case OPCode::OP_LOAD_LOCAL:
    case OPCode::OP_CALL:
      return expect_size(1);
    case OPCode::OP_ADD:
    case OPCode::OP_RETURN:
    case OPCode::OP_PUSH_FRAME:
    case OPCode::OP_POP_FRAME:
      return expect_size(0);
    default:
      return false;
  }
}

}  // namespace Startear