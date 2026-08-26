#include "compiler.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <unordered_map>

namespace tinyscript {
namespace {
struct Token { enum Kind { Word, Number, String, Symbol, End } kind; std::string text; int line; };

std::vector<Token> lex(const std::string& source, std::vector<Diagnostic>& errors) {
    std::vector<Token> out;
    int line = 1;
    for (std::size_t i = 0; i < source.size();) {
        char c = source[i];
        if (c == '\n') { ++line; ++i; continue; }
        if (std::isspace(static_cast<unsigned char>(c))) { ++i; continue; }
        if (c == '#') { while (i < source.size() && source[i] != '\n') ++i; continue; }
        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            std::size_t s = i++; while (i < source.size() && (std::isalnum(static_cast<unsigned char>(source[i])) || source[i] == '_')) ++i;
            out.push_back({Token::Word, source.substr(s, i-s), line}); continue;
        }
        if (std::isdigit(static_cast<unsigned char>(c)) || (c == '.' && i+1 < source.size() && std::isdigit(static_cast<unsigned char>(source[i+1])))) {
            std::size_t s = i++; while (i < source.size() && (std::isdigit(static_cast<unsigned char>(source[i])) || source[i] == '.')) ++i;
            out.push_back({Token::Number, source.substr(s, i-s), line}); continue;
        }
        if (c == '"') {
            ++i; std::string value; bool closed = false;
            while (i < source.size()) { if (source[i] == '"') { ++i; closed = true; break; } if (source[i] == '\n') ++line; value += source[i++]; }
            if (!closed) errors.push_back({line, "Unterminated string."}); else out.push_back({Token::String, value, line});
            continue;
        }
        std::string two = i+1 < source.size() ? source.substr(i,2) : "";
        if (two == "==" || two == "!=" || two == "<=" || two == ">=") { out.push_back({Token::Symbol,two,line}); i += 2; continue; }
        if (std::string("=+-*/<>(),").find(c) != std::string::npos) { out.push_back({Token::Symbol,std::string(1,c),line}); ++i; continue; }
        errors.push_back({line, std::string("Unexpected character '") + c + "'."}); ++i;
    }
    out.push_back({Token::End,"",line}); return out;
}

int variable(Program& p, std::unordered_map<std::string,int>& vars, const std::string& name) {
    auto it = vars.find(name); if (it != vars.end()) return it->second;
    int id = static_cast<int>(p.variables.size()); p.variables.push_back(name); vars[name] = id; return id;
}

bool number(const Token& t, double& n) { if (t.kind != Token::Number) return false; try { n = std::stod(t.text); return true; } catch (...) { return false; } }
}

CompileResult compile(const std::string& source) {
    CompileResult r; auto tokens = lex(source, r.diagnostics); std::unordered_map<std::string,int> vars;
    std::size_t i = 0;
    auto emit = [&](Op op, int operand=0, double n=0, std::string text="") { r.program.code.push_back({op,operand,n,std::move(text)}); };
    while (i + 1 < tokens.size()) {
        Token t = tokens[i];
        if (t.kind == Token::End) break;
        if (t.kind != Token::Word) { r.diagnostics.push_back({t.line,"Expected a command or variable name."}); ++i; continue; }
        if (t.text == "say") {
            ++i; if (tokens[i].kind == Token::String) { emit(Op::PushText,0,0,tokens[i].text); emit(Op::CallNative,0,0,"say"); ++i; } else { r.diagnostics.push_back({t.line,"say needs text in quotes."}); }
            continue;
        }
        if (t.text == "play" || t.text == "music") {
            std::string cmd=t.text; ++i; if(tokens[i].kind==Token::String){ emit(Op::PushText,0,0,tokens[i].text); emit(Op::CallNative,0,0,cmd); ++i; } else r.diagnostics.push_back({t.line,cmd+" needs a quoted asset name."}); continue;
        }
        if (tokens[i+1].kind == Token::Symbol && tokens[i+1].text == "=") {
            int id = variable(r.program, vars, t.text); i += 2;
            if (tokens[i].kind == Token::Number) { double n; number(tokens[i],n); emit(Op::PushNumber,0,n); ++i; }
            else if (tokens[i].kind == Token::String) { emit(Op::PushText,0,0,tokens[i].text); ++i; }
            else if (tokens[i].kind == Token::Word && (tokens[i].text=="true" || tokens[i].text=="false")) { emit(Op::PushBool,tokens[i].text=="true"); ++i; }
            else { r.diagnostics.push_back({t.line,"Assignments in v0.1 currently require a number, string, or boolean."}); while(tokens[i].kind!=Token::End && tokens[i].line==t.line) ++i; continue; }
            emit(Op::Store,id); continue;
        }
        if (t.text == "move" || t.text == "rotate" || t.text == "scale" || t.text == "destroy" || t.text == "restart" || t.text == "quit") {
            emit(Op::CallNative,0,0,t.text); ++i; while(tokens[i].kind!=Token::End && tokens[i].line==t.line) ++i; continue;
        }
        r.diagnostics.push_back({t.line,"Unknown command '"+t.text+"'."}); ++i; while(tokens[i].kind!=Token::End && tokens[i].line==t.line) ++i;
    }
    emit(Op::Halt); return r;
}

std::string disassemble(const Program& p) {
    std::ostringstream out;
    for (std::size_t i=0;i<p.code.size();++i) { out << i << ": " << static_cast<int>(p.code[i].op); if(!p.code[i].text.empty()) out << " " << p.code[i].text; if(p.code[i].op==Op::PushNumber) out << " " << p.code[i].number; out << '\n'; }
    return out.str();
}
} // namespace tinyscript
