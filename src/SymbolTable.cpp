#include "SymbolTable.h"
#include <stdexcept>

SymbolTable::SymbolTable() {
    scopes.emplace_back(); // Global scope
}

void SymbolTable::Define(const std::string& name) {
    if (scopes.back().count(name)) {
        throw std::runtime_error("Redefinition of variable: " + name);
    }
    scopes.back()[name] = nextOffset;
    nextOffset -= 8;
}

void SymbolTable::DefineGlobal(const std::string& name) {
    if (scopes[0].count(name)) {
        throw std::runtime_error("Redefinition of global variable: " + name);
    }
    scopes[0][name] = 0; // Global variables use direct addressing
    globals[name] = true;
}

void SymbolTable::DefineParameter(const std::string& name, int offset) {
    if (scopes.back().count(name)) {
        throw std::runtime_error("Redefinition of parameter: " + name);
    }
    scopes.back()[name] = offset;
}

int SymbolTable::Get(const std::string& name) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        if (it->count(name)) {
            return (*it)[name];
        }
    }
    throw std::runtime_error("Undefined variable: " + name);
}

bool SymbolTable::IsGlobal(const std::string& name) {
    return globals.count(name) > 0;
}

void SymbolTable::EnterScope() {
    scopes.emplace_back();
    // Reset nextOffset for new scope, or manage it relative to current frame
    // For now, a simple reset. This will need more thought for function calls.
    nextOffset = -8;
}

void SymbolTable::LeaveScope() {
    if (scopes.size() <= 1) {
        throw std::runtime_error("Cannot leave global scope");
    }
    scopes.pop_back();
    // Restore nextOffset from previous scope, or manage it relative to current frame
    // For now, a simple reset. This will need more thought for function calls.
    nextOffset = -8;
}