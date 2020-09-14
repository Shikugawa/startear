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

#include "assert.h"

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

void Program::addInst(OPCode code) {
  instructions_.emplace_back(static_cast<size_t>(code));
}

std::optional<std::pair<OPCode, std::deque<size_t>>> Program::fetchInst() {
  auto consume_entry = consume();
  if (!consume_entry) {
    return std::nullopt;
  }
  auto [opcode, offset] = consume_entry.value();
  std::deque<size_t> operands;
  for (auto i = index_ - offset; i < index_; ++i) {
    operands.emplace_back(instructions_[i]);
  }
  return std::make_pair(opcode, operands);
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

void Program::addFunction(std::string name, size_t args) {
  auto current_top = instructions_.size();
  FunctionMetadata metadata{current_top, args};
  func_metadata_.emplace(std::make_pair(name, metadata));
}

std::optional<Program::FunctionMetadata> Program::getFunction(
    std::string name) {
  auto entry = func_metadata_.find(name);
  if (entry != func_metadata_.end()) {
    return entry->second;
  }
  return std::nullopt;
}

void Program::updateIndex(size_t i) {
  STARTEAR_ASSERT(0 <= i && i < instructions_.size());
  index_ = i;
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

std::optional<std::pair<OPCode, uint8_t>> Program::consume() {
  if (isProgramEnd()) {
    return std::nullopt;
  }
  ++index_;

  auto opcode = static_cast<OPCode>(instructions_[index_ - 1]);
  uint8_t offset = 0;

  switch (opcode) {
      // With no operand
    case OPCode::OP_ADD:
    case OPCode::OP_RETURN:
    case OPCode::OP_PUSH_FRAME:
    case OPCode::OP_POP_FRAME:
      break;
      // With 1 operand
    case OPCode::OP_CALL:
    case OPCode::OP_PUSH:
    case OPCode::OP_PRINT:
    case OPCode::OP_STORE_LOCAL:
    case OPCode::OP_LOAD_LOCAL:
      ++index_;
      offset = 1;
      break;
    default:
      std::cout << "Unsupported instruction" << std::endl;
      break;
  }

  return std::make_pair(opcode, offset);
}
}  // namespace Startear