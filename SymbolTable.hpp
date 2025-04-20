#pragma once

#include <string>
#include <unordered_map>
#include "Enums.hpp"

class SymbolTable {
public:
    SymbolTable();
    void reset();
    void define(std::string name, std::string type, Kind kind);
    int varCount(Kind kind);
    Kind kindOf(std::string name);
    std::string typeOf(std::string name);
    int indexOf(std::string name);
    bool exists(std::string name);
private:
    std::unordered_map<std::string, std::string> nameToType;
    std::unordered_map<std::string, Kind> nameToKind;
    std::unordered_map<std::string, int> nameToIndex;

    int staticIndex;
    int fieldIndex;
    int argIndex;
    int varIndex;
};