#pragma once

#include <filesystem>
#include <fstream>
#include "JackTokenizer.hpp"
#include "Enums.hpp"
#include "SymbolTable.hpp"
#include "VMWriter.hpp"

class CompilationEngine {
public:
    // Takes path to single .jack file, and one path to .vm file to write translated code
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

    int labelNumber; // for unique labels in IF, WHILE
    std::string currentClass;

    void writeKeyWord();
    void writeSymbol();
    void writeIntConst();
    void writeStrConst();
    void writeIdentifier();
    void writeType();
    void writeKeyWordConst();
    void compileCurrentObjectSubroutineCall(std::string name);
    void compileClassVarSubroutineCall(std::string name);
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