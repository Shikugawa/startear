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

#define TERMINATE_VM                     \
  state_ = VMState::TerminatedWithError; \
  NOT_REACHED;

namespace Startear {

VMImpl::VMImpl(Program& program) : program_(program) {
  auto main_entry_info =
      program_.functionRegistry().findByName(startup_entry.data());
  if (!main_entry_info.has_value()) {
    std::cerr << "Failed to load `main` function" << std::endl;
    NOT_REACHED;
  }
  pc_ = main_entry_info->get().pc_;
  pushFrame();  // Main Frame
}

void VMImpl::start() {
  while (true) {
    auto instr_entry = program_.fetchInst(pc_);
    if (!instr_entry.has_value() || frame_.size() < 1) {
      break;
    }

    const auto& instr = instr_entry.value();
    auto opcode = instr.get().opcode();
    const auto& operand_ptrs = instr.get().operandsPointer();

    switch (opcode) {
      case OPCode::OP_PRINT: {
        STARTEAR_ASSERT(operand_ptrs.size() == 1);
        auto data_entry = program_.fetchValue(operand_ptrs[0]);
        if (!data_entry || data_entry->category() != Value::Category::Literal) {
          NOT_REACHED;
        }
        print(*data_entry);
        incPc();
        break;
      }
      case OPCode::OP_PUSH: {
        STARTEAR_ASSERT(operand_ptrs.size() == 1);
        auto data_entry = program_.fetchValue(operand_ptrs[0]);
        if (!data_entry || data_entry->category() != Value::Category::Literal) {
          TERMINATE_VM;
        }
        pushStack(*data_entry);
        incPc();
        break;
      }
      case OPCode::OP_ADD: {
        STARTEAR_ASSERT(operand_ptrs.size() == 0);
        add();
        incPc();
        break;
      }
      case OPCode::OP_STORE_LOCAL: {
        STARTEAR_ASSERT(operand_ptrs.size() == 1);
        Value stack_top = popStack();
        auto variable_name_entry = program_.fetchValue(operand_ptrs[0]);
        if (!variable_name_entry || !variable_name_entry->getString()) {
          TERMINATE_VM;
        }
        auto variable_name = *variable_name_entry->getString();
        saveLocalVariableTable(variable_name, stack_top);
        incPc();
        break;
      }
      case OPCode::OP_LOAD_LOCAL: {
        STARTEAR_ASSERT(operand_ptrs.size() == 1);
        auto value_entry = lookupLocalVariableTable(operand_ptrs[0]);
        if (!value_entry) {
          TERMINATE_VM;
        }
        pushStack(*value_entry);
        incPc();
        break;
      }
      case OPCode::OP_RETURN: {
        auto return_pc = frame_.top().return_pc_;
        pc_ = return_pc;
        // Call if the function has no instructions.
        if (frame_.top().stack_.size() == 0) {
          popFrame();
          break;
        }
        auto return_value = popStack();
        popFrame();
        pushStack(return_value);
        break;
      }
      case OPCode::OP_BANG_EQUAL:
      case OPCode::OP_GREATER_EQUAL:
      case OPCode::OP_LESS_EQUAL:
      case OPCode::OP_LESS:
      case OPCode::OP_GREATER:
      case OPCode::OP_EQUAL: {
        STARTEAR_ASSERT(operand_ptrs.size() == 0);
        if (frame_.top().stack_.size() < 2) {
          TERMINATE_VM;
        }
        auto lhs = popStack();
        auto rhs = popStack();
        if (!lhs.getDouble() || !rhs.getDouble()) {
          TERMINATE_VM;
        }
        bool result = cmp(opcode, *lhs.getDouble(), *rhs.getDouble());
        pushStack(Value(Value::Category::Literal, static_cast<double>(result)));
        incPc();
        break;
      }
      case OPCode::OP_BRANCH: {
        STARTEAR_ASSERT(operand_ptrs.size() == 2);
        bool cmp = static_cast<bool>(*popStack().getDouble());
        auto label_entry = program_.fetchValue(cmp ? operand_ptrs[0] : operand_ptrs[1]);
        if (!label_entry.has_value() || !label_entry->getString()) {
          TERMINATE_VM;
        }
        auto metadata_entry =
            program_.functionRegistry().findByName(*label_entry->getString());
        if (!metadata_entry) {
          std::cerr << fmt::format("Failed to find label entry on {}",
                                   *label_entry->getString())
                    << std::endl;
          TERMINATE_VM;
        }
        auto pc = metadata_entry->get().pc_;
        if (pc < 0 || pc >= program_.instructions().size()) {
          state_ = VMState::SuccessfulTerminated;
          return;
        }
        pc_ = pc;
        break;
      }
      case OPCode::OP_CALL: {
        STARTEAR_ASSERT(operand_ptrs.size() == 1);
        auto func_label_entry = program_.fetchValue(operand_ptrs[0]);
        if (!func_label_entry) {
          TERMINATE_VM;
        }
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
          TERMINATE_VM;
        }

        Frame next_frame;
        next_frame.return_pc_ = pc_ + 1;

        // Extract stack value from current frame to next one.
        for (int32_t /* not to be inferenced as unsigned integer */ i =
                 func_entry->get().args_.size() - 1;
             i >= 0; --i) {
          auto current_stack_top = popStack();
          next_frame.stack_.push(current_stack_top);
          auto arg_name_entry = program_.fetchValue(func_entry->get().args_[i]);
          if (!arg_name_entry.has_value()) {
            std::cerr << "The variable name of argument is not registered on "
                         "program data region"
                      << std::endl;
            TERMINATE_VM;
          }
          if (!arg_name_entry->getString().has_value()) {
            TERMINATE_VM;
          }
          next_frame.lv_table_.emplace(*arg_name_entry->getString(),
                                       current_stack_top);
        }

        pc_ = func_entry->get().pc_;
        frame_.emplace(next_frame);
        break;
      }
      default:
        std::cerr << fmt::format("{} is unsupported instruction",
                                 opcodeToString(opcode))
                  << std::endl;
        TERMINATE_VM;
    }
  }

  /**
   * TODO: We should pop main frame when execution is finished.
   * In this implementation, we remain it to analyse frame state on test.
   * Add hooking strategy to check them at test time.
   */
  state_ = VMState::SuccessfulTerminated;
}

void VMImpl::restart(Program& program) {
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
    return lookupLocalVariableTable(*variable_name_entry->getString());
  }
  return std::nullopt;
}

std::optional<Value> VMImpl::lookupLocalVariableTable(
    std::string variable_name) {
  auto variable_itr = frame_.top().lv_table_.find(variable_name);
  if (variable_itr == frame_.top().lv_table_.end()) {
    return std::nullopt;
  }
  auto literal = variable_itr->second;
  STARTEAR_ASSERT(literal.category() == Value::Category::Literal);
  return literal;
}

void VMImpl::saveLocalVariableTable(std::string name, Value& v) {
  auto entry = frame_.top().lv_table_.find(name);
  if (entry != frame_.top().lv_table_.end()) {
    entry->second = v;
  } else {
    frame_.top().lv_table_.emplace(name, v);
  }
}

void VMImpl::print(Value& v) {
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
      TERMINATE_VM;
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

bool VMImpl::cmp(OPCode code, double lhs, double rhs) {
  switch (code) {
    case OPCode::OP_BANG_EQUAL:
      return lhs != rhs;
    case OPCode::OP_GREATER_EQUAL:
      return lhs <= rhs;
    case OPCode::OP_LESS_EQUAL:
      return lhs >= rhs;
    case OPCode::OP_LESS:
      return lhs > rhs;
    case OPCode::OP_GREATER:
      return lhs < rhs;
    case OPCode::OP_EQUAL:
      return lhs == rhs;
    default:
      NOT_REACHED;
  }
}

}  // namespace Startear