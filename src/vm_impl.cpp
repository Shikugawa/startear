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

#include "vm_impl.h"

#include <fmt/format.h>

#include <iostream>

namespace Startear {
void VMImpl::start() {
  auto instr_entry = program_.fetchInst();
  incPc();

  while (instr_entry) {
    auto [opcode, operand_ptrs] = instr_entry.value();
    switch (opcode) {
      case OPCode::OP_PRINT: {
        STARTEAR_ASSERT(operand_ptrs.size() == 1);
        auto data_entry = program_.fetchValue(operand_ptrs[0]);
        if (data_entry) {
          STARTEAR_ASSERT(data_entry->category() == Value::Category::Literal);
          print(*data_entry);
        }
        break;
      }
      case OPCode::OP_PUSH: {
        STARTEAR_ASSERT(operand_ptrs.size() == 1);
        auto data_entry = program_.fetchValue(operand_ptrs[0]);
        if (data_entry) {
          STARTEAR_ASSERT(data_entry->category() == Value::Category::Literal);
          pushStack(*data_entry);
        } else {
          NOT_REACHED;
        }
        break;
      }
      case OPCode::OP_ADD: {
        STARTEAR_ASSERT(operand_ptrs.size() == 0);
        add();
        break;
      }
      case OPCode::OP_STORE_LOCAL: {
        STARTEAR_ASSERT(operand_ptrs.size() == 1);
        Value stack_top = popStack();
        auto variable_name_entry = program_.fetchValue(operand_ptrs[0]);
        if (variable_name_entry) {
          STARTEAR_ASSERT(variable_name_entry->getString().has_value());
          auto variable_name = *variable_name_entry->getString();
          saveLocalVariableTable(variable_name, stack_top);
        }
        break;
      }
      case OPCode::OP_LOAD_LOCAL: {
        STARTEAR_ASSERT(operand_ptrs.size() == 1);
        auto value_entry = lookupLocalVariableTable(operand_ptrs[0]);
        if (value_entry) {
          pushStack(*value_entry);
        }
        break;
      }
      case OPCode::OP_PUSH_FRAME: {
        STARTEAR_ASSERT(operand_ptrs.size() == 1);
        auto data_entry = program_.fetchValue(operand_ptrs[0]);
        if (data_entry) {
          STARTEAR_ASSERT(data_entry->category() == Value::Category::Literal);
          pushFrame(static_cast<size_t>(*data_entry->getDouble()));
        } else {
          NOT_REACHED;
        }
        break;
      }
      case OPCode::OP_RETURN: {
        auto return_pc = frame_.top().return_pc_;
        popFrame();
        pc_ = return_pc;
        program_.updateIndex(return_pc);
      }
      case OPCode::OP_CALL: {
        STARTEAR_ASSERT(operand_ptrs.size() == 1);
        auto func_label_entry = program_.fetchValue(operand_ptrs[0]);
        if (func_label_entry) {
          STARTEAR_ASSERT(func_label_entry->category() ==
                          Value::Category::Variable);
          STARTEAR_ASSERT(func_label_entry->getString());
          // TODO: This information should be known when code analysis phase
          auto func_entry = program_.functionRegistry().findByName(
              *func_label_entry->getString());
          if (!func_entry.has_value()) {
            std::cerr << fmt::format("{} is not defined",
                                     *func_label_entry->getString())
                      << std::endl;
            state_ = VMState::TerminatedWithError;
            return;
          }
          frame_.top().return_pc_ = pc_ + 1;
          pc_ = func_entry->get().pc_;
          program_.updateIndex(func_entry->get().pc_);
        }
      }
      default:
        std::cerr << fmt::format("{} is unsupported instruction", opcode)
                  << std::endl;
        state_ = VMState::TerminatedWithError;
        return;
    }
    instr_entry = program_.fetchInst();
  }
  state_ = VMState::SuccessfulTerminated;
}

void VMImpl::restart(Program &program) {
  STARTEAR_ASSERT(state_ == VMState::SuccessfulTerminated ||
                  state_ == VMState::TerminatedWithError);
  program_ = program;
  state_ = VMState::Initialized;
  start();
}

std::optional<Value> VMImpl::lookupLocalVariableTable(size_t ptr) {
  auto variable_name_entry = program_.fetchValue(ptr);
  if (!variable_name_entry) {
    return std::nullopt;
  }
  STARTEAR_ASSERT(variable_name_entry->category() == Value::Category::Variable);
  if (variable_name_entry->getString()) {
    auto variable_name = *variable_name_entry->getString();
    auto variable_itr = frame_.top().lv_table_.find(variable_name);
    if (variable_itr == frame_.top().lv_table_.end()) {
      return std::nullopt;
    }
    auto literal = program_.fetchValue(variable_itr->second);
    STARTEAR_ASSERT(literal->category() == Value::Category::Literal);
    if (!literal) {
      return std::nullopt;
    }
    return *literal;
  }
  return std::nullopt;
}

void VMImpl::saveLocalVariableTable(std::string name, Value v) {
  frame_.top().lv_table_[name] = program_.addValue(v);
}

void VMImpl::print(Value v) {
  switch (v.type()) {
    case Value::SupportedTypes::String:
      if (!v.getString()) NOT_REACHED;
      std::cout << v.getString().value() << std::endl;
      break;
    case Value::SupportedTypes::Double:
      if (!v.getDouble()) NOT_REACHED;
      std::cout << v.getDouble().value() << std::endl;
      break;
    default:
      NOT_REACHED;
  }
}

void VMImpl::add() {
  auto operand1 = getStackTop();
  if (operand1.type() != Value::SupportedTypes::Double) {
    return;
  }
  popStack();
  auto operand2 = getStackTop();
  if (operand2.type() != Value::SupportedTypes::Double) {
    return;
  }
  popStack();
  auto result = operand1.getDouble().value() + operand2.getDouble().value();
  Value v(Value::Category::Literal, result);
  pushStack(v);
}

}  // namespace Startear