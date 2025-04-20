
#include "SymbolTable.hpp"
#include "Enums.hpp"
#include <string>
#include <unordered_map>
#include <stdexcept>

SymbolTable::SymbolTable() {
    reset();
}

void SymbolTable::reset() {
    nameToType.clear();
    nameToKind.clear();
    staticIndex = 0;
    fieldIndex = 0;
    argIndex = 0;
    varIndex = 0;   
}

void SymbolTable::define(std::string name, std::string type, Kind kind) {
    nameToType[name] = type;
    nameToKind[name] = kind;
    if (kind == NONE) {
        throw std::runtime_error("Cannot add a variable of kind NONE to symbol table.");
    }
    else if (kind == STATIC) {
        nameToIndex[name] = staticIndex++;
    }
    else if (kind == FIELD) {
        nameToIndex[name] = fieldIndex++;
    }   
    else if (kind == ARG) {
        nameToIndex[name] = argIndex++;
    }
    else if (kind == VAR) {
        nameToIndex[name] = varIndex++;
    }
}

int SymbolTable::varCount(Kind kind) {
    if (kind == STATIC) return staticIndex;
    if (kind == FIELD) return fieldIndex;
    if (kind == ARG) return argIndex;
    if (kind == VAR) return varIndex;
    return 0;
}

Kind SymbolTable::kindOf(std::string name) {
    if (nameToKind.find(name) == nameToKind.end()) {
        return NONE;
    }
    return nameToKind[name];
}

int SymbolTable::indexOf(std::string name) {
    if (nameToIndex.find(name) == nameToIndex.end()) {
        throw std::runtime_error("Tried to find index of an identifier which doesn't exist!");
    }
    return nameToIndex[name];
}

std::string SymbolTable::typeOf(std::string name) {
    if (nameToType.find(name) == nameToType.end()) {
        throw std::runtime_error("Tried to find index of an identifier which doesn't exist!");
    }
    return nameToType[name];
}

bool SymbolTable::exists(std::string name) {
    return (nameToKind.find(name) != nameToKind.end());
}
