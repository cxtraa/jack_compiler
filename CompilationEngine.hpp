#pragma once

#include <filesystem>
#include <fstream>
#include "JackTokenizer.hpp"
#include "Enums.hpp"
#include "SymbolTable.hpp"
#include "VMWriter.hpp"

class CompilationEngine {
public:
    CompilationEngine(std::filesystem::path inputPath, std::filesystem::path outputPath);

    void compileClass();
    void compileClassVarDec();
    void compileSubroutineDec();
    int compileParameterList();
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
    int compileExpressionList();

private:
    JackTokenizer tokenizer;

    SymbolTable classSymbolTable;
    SymbolTable subroutineSymbolTable;

    VMWriter vmWriter;

    std::ifstream inputStream;

    int labelNumber;
    std::string currentClass;

    void writeKeyWord();
    void writeSymbol();
    void writeIntConst();
    void writeStrConst();
    void writeVar(std::string type, Kind kind, bool isDeclaration, bool isClass);
    void writeIdentifier();
    void writeType();
    void writeKeyWordConst();
    bool isType();
    bool isStatement();
    bool isTerm();
    bool isKeyWordConstant();
    bool isOp();
    bool isUnaryOp();
    std::string binaryOp();
    std::string unaryOp();
    std::string kindToStr(Kind kind);
    Kind kindOf(std::string name);
    int indexOf(std::string name);
    std::string typeOf(std::string name);

    std::unordered_map<KeyWord, std::string> keyWordToStr;
};