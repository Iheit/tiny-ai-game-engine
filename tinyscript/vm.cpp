#include "vm.hpp"
#include <cmath>
#include <sstream>

namespace tinyscript {
namespace {
bool truthy(const Value& v) { if (auto b=std::get_if<bool>(&v)) return *b; if (auto n=std::get_if<double>(&v)) return *n != 0.0; if (auto s=std::get_if<std::string>(&v)) return !s->empty(); return false; }
double num(const Value& v) { return std::get<double>(v); }
}
void VM::set_native(const std::string& name, NativeFunction fn) { natives_[name] = std::move(fn); }
bool VM::run(const Program& p, std::string& error) {
    std::vector<Value> stack; std::size_t ip=0;
    while (ip < p.code.size()) {
        const auto& ins=p.code[ip++];
        try {
            switch(ins.op) {
            case Op::Halt: return true;
            case Op::PushNumber: stack.emplace_back(ins.number); break;
            case Op::PushText: stack.emplace_back(ins.text); break;
            case Op::PushBool: stack.emplace_back(ins.operand != 0); break;
            case Op::Load: { auto it=globals_.find(p.variables.at(ins.operand)); stack.emplace_back(it==globals_.end()?Value{}:it->second); break; }
            case Op::Store: { if(stack.empty()){error="Stack underflow while storing variable.";return false;} globals_[p.variables.at(ins.operand)]=stack.back(); stack.pop_back(); break; }
            case Op::Add: { auto b=stack.back();stack.pop_back();auto a=stack.back();stack.pop_back(); if(auto sa=std::get_if<std::string>(&a)) { auto sb=std::get_if<std::string>(&b); if(!sb){error="Cannot add text and a non-text value.";return false;} stack.emplace_back(*sa+*sb); } else stack.emplace_back(num(a)+num(b)); break; }
            case Op::Sub: {auto b=stack.back();stack.pop_back();auto a=stack.back();stack.pop_back();stack.emplace_back(num(a)-num(b));break;}
            case Op::Mul: {auto b=stack.back();stack.pop_back();auto a=stack.back();stack.pop_back();stack.emplace_back(num(a)*num(b));break;}
            case Op::Div: {auto b=stack.back();stack.pop_back();auto a=stack.back();stack.pop_back();if(num(b)==0){error="Division by zero.";return false;}stack.emplace_back(num(a)/num(b));break;}
            case Op::CallNative: {auto it=natives_.find(ins.text);if(it==natives_.end()){error="Unknown native command: "+ins.text;return false;}std::vector<Value> args;if(!stack.empty()){args.push_back(stack.back());stack.pop_back();}it->second(args);break;}
            case Op::Pop: if(!stack.empty())stack.pop_back(); break;
            default: error="Opcode is not implemented in TinyScript v0.1."; return false;
            }
        } catch(const std::exception& e) { error=e.what(); return false; }
    }
    return true;
}
}
