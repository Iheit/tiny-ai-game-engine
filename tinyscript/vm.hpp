#pragma once
#include "compiler.hpp"
#include <functional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace tinyscript {
using Value = std::variant<std::monostate, double, std::string, bool>;
using NativeFunction = std::function<void(const std::vector<Value>&)>;

class VM {
public:
    void set_native(const std::string& name, NativeFunction fn);
    bool run(const Program& program, std::string& error);
    const std::unordered_map<std::string, Value>& globals() const { return globals_; }
private:
    std::unordered_map<std::string, NativeFunction> natives_;
    std::unordered_map<std::string, Value> globals_;
};
}
