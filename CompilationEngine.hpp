#pragma once

#include <filesystem>
#include <fstream>
#include "JackTokenizer.hpp"
#include "Enums.hpp"
#include "SymbolTable.hpp"

class CompilationEngine {
public:
    CompilationEngine(std::filesystem::path inputPath, std::filesystem::path outputPath);

    void compileClass();
    void compileClassVarDec();
    void compileSubroutineDec();
    void compileParameterList();
    void compileSubroutineBody();
    void compileVarDec();
    void compileStatements();
    void compileLet();
    void compileIf();
    void compileWhile();
    void compileDo();
    void compileReturn();
    void compileSubroutineCall();
    void compileExpression();
    void compileTerm();
    void compileExpressionList();

private:
    JackTokenizer tokenizer;

    SymbolTable classSymbolTable;
    SymbolTable subroutineSymbolTable;

    std::ifstream inputStream;
    std::ofstream outputStream;

    void writeKeyWord();
    void writeSymbol();
    void writeIntConst();
    void writeStrConst();
    void writeVar(std::string type, Kind kind, bool isDeclaration, bool isClass);
    void writeType();
    bool isType();
    bool isStatement();
    bool isTerm();
    bool isKeyWordConstant();
    bool isOp();
    bool isUnaryOp();
    std::string kindToStr(Kind kind);
    Kind kindOf(std::string name);
    int indexOf(std::string name);
    std::string typeOf(std::string name);

    std::unordered_map<KeyWord, std::string> keyWordToStr;
};