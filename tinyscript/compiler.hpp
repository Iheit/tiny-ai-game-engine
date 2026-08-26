#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace tinyscript {

enum class Op : std::uint8_t {
    Halt, PushNumber, PushText, PushBool, Load, Store, Add, Sub, Mul, Div,
    Equal, NotEqual, Less, LessEqual, Greater, GreaterEqual, JumpIfFalse,
    Jump, CallNative, Pop
};

struct Instruction {
    Op op{};
    std::int32_t operand = 0;
    double number = 0.0;
    std::string text;
};

struct Program {
    std::vector<Instruction> code;
    std::vector<std::string> variables;
};

struct Diagnostic {
    int line = 0;
    std::string message;
};

struct CompileResult {
    Program program;
    std::vector<Diagnostic> diagnostics;
    bool ok() const { return diagnostics.empty(); }
};

CompileResult compile(const std::string& source);
std::string disassemble(const Program& program);

} // namespace tinyscript
