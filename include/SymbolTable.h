
#pragma once

#include <vector>
#include <string>
#include <unordered_map>

class SymbolTable {
public:
    SymbolTable();
    void Define(const std::string& name);
    void DefineGlobal(const std::string& name);
    void DefineParameter(const std::string& name, int offset);
    int Get(const std::string& name);
    bool IsGlobal(const std::string& name);
    void EnterScope();
    void LeaveScope();

private:
    std::vector<std::unordered_map<std::string, int>> scopes;
    std::unordered_map<std::string, bool> globals;
    int nextOffset = -8; // Start at [rbp-8]
};
