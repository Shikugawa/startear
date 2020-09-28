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

#ifndef STARTEAR_ALL_DISASSEMBLER_H
#define STARTEAR_ALL_DISASSEMBLER_H

#include <fmt/format.h>

#include "opcode.h"
#include "program.h"
#include "startear_assert.h"

#define SET_INSTRUCTION(x)     \
  if (instr_str.size() == 0) { \
    instr_str = x;             \
  }

namespace Startear {

void disassemble(Program& p) {
  size_t ptr = 0;
  while (true) {
    auto instr_entry = p.fetchInst(ptr);
    if (!instr_entry.has_value()) {
      break;
    }

    std::string instr_str;
    auto func_name = p.functionRegistry().findByProgramCounter(ptr);

    switch (instr_entry->get().opcode()) {
      case OPCode::OP_ADD:
        instr_str = "OP_ADD";
      case OPCode::OP_PUSH_FRAME:
        SET_INSTRUCTION("OP_PUSH_FRAME");
      case OPCode::OP_POP_FRAME:
        SET_INSTRUCTION("OP_POP_FRAME")
      case OPCode::OP_RETURN:
        SET_INSTRUCTION("OP_RETURN");
        std::cout << fmt::format("{}", instr_str);
        if (func_name.has_value()) {
          std::cout << fmt::format(" <- {}", func_name.value().get().name_);
        }
        std::cout << std::endl;
        break;
      case OPCode::OP_LOAD_LOCAL:
        SET_INSTRUCTION("OP_LOAD_LOCAL");
      case OPCode::OP_CALL:
        SET_INSTRUCTION("OP_CALL")
      case OPCode::OP_STORE_LOCAL: {
        SET_INSTRUCTION("OP_STORE_LOCAL");
        auto operand_ptrs = instr_entry->get().operandsPointer();
        STARTEAR_ASSERT(operand_ptrs.size() == 1);
        auto operand_data_entry = p.fetchValue(operand_ptrs[0]);
        std::cout << fmt::format("{} {}", instr_str,
                                 operand_data_entry->getString().value());
        if (func_name.has_value()) {
          std::cout << fmt::format(" <- {}", func_name.value().get().name_);
        }
        std::cout << std::endl;
        break;
      }
      case OPCode::OP_PUSH:
        SET_INSTRUCTION("OP_PUSH");
      case OPCode::OP_PRINT: {
        SET_INSTRUCTION("OP_PRINT");
        auto operand_ptrs = instr_entry->get().operandsPointer();
        STARTEAR_ASSERT(operand_ptrs.size() == 1);
        auto operand_data_entry = p.fetchValue(operand_ptrs[0]);
        if (operand_data_entry) {
          switch (operand_data_entry->type()) {
            case Value::SupportedTypes::String:
              std::cout << fmt::format("{} {}", instr_str,
                                       operand_data_entry->getString().value());
              break;
            case Value::SupportedTypes::Double:
              std::cout << fmt::format("{} {}", instr_str,
                                       operand_data_entry->getDouble().value());
              break;
            default:
              NOT_REACHED;
          }
          if (func_name.has_value()) {
            std::cout << fmt::format(" <- {}", func_name.value().get().name_);
          }
          std::cout << std::endl;
        }
        break;
      }
    }
    ++ptr;
  }
}

}  // namespace Startear

#endif  // STARTEAR_ALL_DISASSEMBLER_H
