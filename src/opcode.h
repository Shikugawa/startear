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

#ifndef STARTEAR_ALL_OPCODE_H
#define STARTEAR_ALL_OPCODE_H

#include <string>

namespace Startear {

enum class OPCode : size_t {
  /**
   * Print a operand
   * e.g.
   * OP_PRINT 1.0
   * OP_PRINT "sample"
   */
  OP_PRINT,
  /**
   * Push a operand to the top of stack.
   * e.g. OP_PUSH 3.0
   */
  OP_PUSH,
  /**
   * Pop two values from the stack, and add them. Finally, store calculation
   * result
   */
  OP_ADD,
  /**
   * Save stack top value with operand variable name.
   * e.g. OP_STORE_LOCAL "n"
   */
  OP_STORE_LOCAL,
  /**
   * Look up specified variable and push top of stack.
   * e.g. OP_LOAD_LOCAL "n"
   */
  OP_LOAD_LOCAL,
  /**
   * Call function.
   * e.g. OP_CALL "sub"
   *
   * Create frame. frame is defined as the unit of scope.
   * In general, first operand is used to specify return point.
   * e.g.
   *
   * 32 | OP_RETURN # pop frame. If there is returning value.
   *                # VM will drain top value of current stack,
   *                # and push it into the stack on previous frame.
   *                # It will recover program counter from current frame.
   *
   * 42 | OP_PUSH 32         # first argument
   * 43 | OP_CALL "sub"      # function call. It will set next pointer to return
   * back.
   * 44 | OP_STORE_LOCAL "n" # Store returned value from function "sub"
   *
   * Note that only to use this instruction without operands will be treated as
   * startup function like "main".
   */
  OP_CALL,
  OP_PUSH_FRAME,  // deprecated
  OP_POP_FRAME,   // deprecated
  /**
   * Return the stack ptr and program counter. If there is return values
   * , store them to the top of stack. It is not supported to return multiple
   * values.
   */
  OP_RETURN,
};

std::string opcodeToString(OPCode op);

}  // namespace Startear

#endif  // STARTEAR_ALL_OPCODE_H
