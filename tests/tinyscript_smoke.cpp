#include "compiler.hpp"
#include <iostream>
int main(){
    const char* source = "health = 100\nsay \"hello\"\n";
    auto result = tinyscript::compile(source);
    if(!result.ok()){
        for(const auto& d: result.diagnostics) std::cerr << d.line << ": " << d.message << "\n";
        return 1;
    }
    if(result.program.code.empty()) return 2;
    return 0;
}
