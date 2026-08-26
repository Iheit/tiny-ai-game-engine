#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
namespace tiny::script {
struct Diagnostic{int line=0;std::string message;};
enum class Op{PushNumber,PushString,PushBool,Load,Store,Add,Sub,Mul,Div,Equal,Less,Greater,JumpFalse,Jump,Native,Halt};
struct Instruction{Op op{};double number=0;std::string text;int arg=0;};
struct Program{std::vector<Instruction> code;std::vector<std::string> variables;};
struct CompileResult{Program program;std::vector<Diagnostic> errors;bool ok()const{return errors.empty();}};
CompileResult compile(const std::string& source);
class VM{public:using Native=std::function<void(const std::string&,const std::vector<std::string>&)>;void setNative(Native n){native_=std::move(n);}bool run(const Program&,int budget=10000);private:Native native_;std::unordered_map<std::string,double> vars_;std::vector<std::string> stack_;};
}
