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

#include "opcode.h"

#include <string>

namespace Startear {
std::string opcodeToString(OPCode op) {
  switch (op) {
    case OPCode::OP_PRINT:
      return "OP_PRINT";
    case OPCode::OP_PUSH:
      return "OP_PUSH";
    case OPCode::OP_ADD:
      return "OP_ADD";
    case OPCode::OP_STORE_LOCAL:
      return "OP_STORE_LOCAL";
    case OPCode::OP_LOAD_LOCAL:
      return "OP_LOAD_LOCAL";
    case OPCode::OP_CALL:
      return "OP_CALL";
    case OPCode::OP_PUSH_FRAME:
      return "OP_PUSH_FRAME";
    case OPCode::OP_POP_FRAME:
      return "OP_POP_FRAME";
    case OPCode::OP_RETURN:
      return "OP_RETURN";
    default:
      return "";
  }
}

}  // namespace Startear