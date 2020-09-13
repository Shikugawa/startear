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

#include "assert.h"
#include "opcode.h"
#include "program.h"

#define SET_INSTRUCTION(x)     \
  if (instr_str.size() == 0) { \
    instr_str = x;             \
  }

namespace Startear {

void disassemble(Program& p) {
  size_t ptr = 0;
  while (ptr < p.instructions().size()) {
    auto instr = static_cast<OPCode>(p.instructions()[ptr]);
    std::string instr_str;
    switch (instr) {
      case OPCode::OP_ADD:
        instr_str = "OP_ADD";
      case OPCode::OP_RETURN:
        SET_INSTRUCTION("OP_RETURN");
        std::cout << fmt::format("{}", instr_str) << std::endl;
        break;
      case OPCode::OP_LOAD_LOCAL:
        SET_INSTRUCTION("OP_LOAD_LOCAL");
      case OPCode::OP_STORE_LOCAL: {
        SET_INSTRUCTION("OP_STORE_LOCAL");
        ++ptr;
        auto operand_ptr = p.instructions()[ptr];
        auto operand_data_entry = p.fetchValue(operand_ptr);
        std::cout << fmt::format("{} {}", instr_str,
                                 operand_data_entry->getString().value())
                  << std::endl;
        break;
      }
      case OPCode::OP_PUSH:
        SET_INSTRUCTION("OP_PUSH");
      case OPCode::OP_PUSH_FRAME:
        SET_INSTRUCTION("OP_PUSH_FRAME");
      case OPCode::OP_PRINT: {
        SET_INSTRUCTION("OP_PRINT");
        ++ptr;
        auto operand_ptr = p.instructions()[ptr];
        auto operand_data_entry = p.fetchValue(operand_ptr);
        if (operand_data_entry) {
          switch (operand_data_entry->type()) {
            case Value::SupportedTypes::String:
              std::cout << fmt::format("{} {}", instr_str,
                                       operand_data_entry->getString().value())
                        << std::endl;
              break;
            case Value::SupportedTypes::Double:
              std::cout << fmt::format("{} {}", instr_str,
                                       operand_data_entry->getDouble().value())
                        << std::endl;
              break;
            default:
              NOT_REACHED;
          }
        }
        break;
      }
    }
    ++ptr;
  }
}

}  // namespace Startear

#endif  // STARTEAR_ALL_DISASSEMBLER_H
