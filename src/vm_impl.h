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

#ifndef STARTEAR_ALL_VM_IMPL_H
#define STARTEAR_ALL_VM_IMPL_H

#include <stack>
#include <string_view>

#include "opcode.h"
#include "vm.h"

namespace Startear {
namespace {
static constexpr std::string_view startup_entry = "main";
}

class VMImpl : public VM {
 public:
  VMImpl(Program& program);

  // VM
  void incPc() override { ++pc_; }
  void pushStack(Value v) override {
    STARTEAR_ASSERT(frame_.size() != 0);
    frame_.top().stack_.push(v);
  }
  Value getStackTop() {
    STARTEAR_ASSERT(frame_.size() != 0);
    return frame_.top().stack_.top();
  }
  Value popStack() override {
    STARTEAR_ASSERT(frame_.size() != 0);
    auto top = frame_.top().stack_.top();
    frame_.top().stack_.pop();
    return top;
  }

  void pushFrame(size_t return_pc) {
    Frame f;
    f.return_pc_ = return_pc;
    frame_.push(f);
  }
  void pushFrame() {
    // Only to call at once.
    if (frame_.size() != 0) {
      state_ = VMState::TerminatedWithError;
      std::cerr << "Only to call push frame at once without return value";
      NOT_REACHED;
    }
    Frame f;
    frame_.push(f);
  }
  void popFrame() {
    auto return_pc = frame_.top().return_pc_;
    pc_ = return_pc;
    frame_.pop();
  }
  void start();
  void restart(Program& program);

 private:
  enum VMState {
    // Default state. Program has set already,
    // and all of properties of instructions are initialized.
    Initialized,
    // All of programs are run out successfully.
    SuccessfulTerminated,
    // VM is terminated with some of error.
    TerminatedWithError,
  };

  // It determines the scope of program.
  // We assume that this is used as stack way.
  struct Frame {
    std::stack<Value> stack_;                           // Execution stack
    std::unordered_map<std::string, size_t> lv_table_;  // Local variable table
    // Program counter which is used to point out the place of memory.
    size_t return_pc_{0};
  };

  std::optional<Value> lookupLocalVariableTable(size_t ptr);
  void saveLocalVariableTable(std::string name, Value v);
  void print(Value v);
  void add();

  size_t pc_{0};      // Program counter
  Program& program_;  // All of codes which will be executed
  std::stack<Frame> frame_;
  VMState state_{VMState::Initialized};
};
}  // namespace Startear

#endif  // STARTEAR_ALL_VM_IMPL_H
