#include "CompilationEngine.hpp"
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <algorithm>

CompilationEngine::CompilationEngine(std::filesystem::path inputPath, std::filesystem::path outputPath) : tokenizer(inputPath), classSymbolTable(), subroutineSymbolTable() {
    inputStream = std::ifstream(inputPath);
    if (!inputStream) throw std::runtime_error("CompilationEngine: the input file could not be read from.");
    outputStream = std::ofstream(outputPath); 
    if (!outputStream) throw std::runtime_error("CompilationEngine: the output file could not be created/written to.");

    keyWordToStr = {
        {KW_CLASS, "class"},
        {KW_METHOD, "method"},
        {KW_FUNCTION, "function"},
        {KW_CONSTRUCTOR, "constructor"},
        {KW_INT, "int"},
        {KW_BOOLEAN, "boolean"},
        {KW_CHAR, "char"},
        {KW_VOID, "void"},
        {KW_VAR, "var"},
        {KW_STATIC, "static"},
        {KW_FIELD, "field"},
        {KW_LET, "let"},
        {KW_DO, "do"},
        {KW_IF, "if"},
        {KW_ELSE, "else"},
        {KW_WHILE, "while"},
        {KW_RETURN, "return"},
        {KW_TRUE, "true"},
        {KW_FALSE, "false"},
        {KW_NULL, "null"},
        {KW_THIS, "this"}
    };

    tokenizer.advance();
}

void CompilationEngine::compileClass() {
    classSymbolTable.reset();
    outputStream << "<class>" << std::endl;
    writeKeyWord();
    outputStream << "<identifier category=\"class\" index=\"-1\" usage=\"declared\">" << tokenizer.identifier() << "</identifier>" << std::endl;
    tokenizer.advance();
    writeSymbol();
    while (tokenizer.tokenType() == KEYWORD && (tokenizer.keyWord() == KW_STATIC || tokenizer.keyWord() == KW_FIELD)) {
        compileClassVarDec();
    }
    while (tokenizer.tokenType() == KEYWORD && (tokenizer.keyWord() == KW_CONSTRUCTOR || tokenizer.keyWord() == KW_FUNCTION || tokenizer.keyWord() == KW_METHOD)) {
        compileSubroutineDec();
    }
    writeSymbol();
    outputStream << "</class>" << std::endl;
}

void CompilationEngine::compileClassVarDec() {
    outputStream << "<classVarDec>" << std::endl;
    Kind varKind = tokenizer.keyWord() == KW_STATIC ? STATIC : FIELD;
    writeKeyWord();
    std::string varType = tokenizer.type();
    writeType();
    writeVar(varType, varKind, true, true);
    while (tokenizer.tokenType() == SYMBOL && tokenizer.symbol() == ',') {
        writeSymbol();
        writeVar(varType, varKind, true, true);
    }
    writeSymbol();
    outputStream << "</classVarDec>" << std::endl;
}

void CompilationEngine::compileSubroutineDec() {
    subroutineSymbolTable.reset();
    outputStream << "<subroutineDec>" << std::endl;
    writeKeyWord(); // constructor / function / method
    if (tokenizer.tokenType() == KEYWORD && tokenizer.keyWord() == KW_VOID) {
        writeKeyWord();
    }
    else {
        writeType();
    }
    outputStream << "<identifier category=\"subroutine\" index=\"-1\" usage=\"declared\">" << tokenizer.identifier() << "</identifier>" << std::endl;
    tokenizer.advance();
    writeSymbol(); // '('
    compileParameterList();
    writeSymbol(); // ')'
    compileSubroutineBody();
    outputStream << "</subroutineDec>" << std::endl;
}

void CompilationEngine::compileParameterList() {
    outputStream << "<parameterList>" << std::endl;
    if (isType()) { // (type varName)
        std::string varType = tokenizer.type();
        writeType();
        writeVar(varType, ARG, true, false);
    }
    while (tokenizer.tokenType() == SYMBOL && tokenizer.symbol() == ',') { // (',' type varName)*
        writeSymbol();
        std::string varType = tokenizer.type();
        writeType();
        writeVar(varType, ARG, true, false);
    }
    outputStream << "</parameterList>" << std::endl;
}

void CompilationEngine::compileSubroutineBody() {
    outputStream << "<subroutineBody>" << std::endl;
    writeSymbol(); // '{'
    while (tokenizer.tokenType() == KEYWORD && tokenizer.keyWord() == KW_VAR) {
        compileVarDec();
    }
    compileStatements();
    writeSymbol(); // '}'
    outputStream << "</subroutineBody>" << std::endl;
}

void CompilationEngine::compileVarDec() {
    outputStream << "<varDec>" << std::endl;
    writeKeyWord(); // 'var'

    std::string type = tokenizer.type();
    writeType(); // type
    writeVar(type, VAR, true, false);

    while (tokenizer.tokenType() == SYMBOL && tokenizer.symbol() == ',') {
        writeSymbol();
        writeVar(type, VAR, true, false);
    }
    writeSymbol(); // ;
    outputStream << "</varDec>" << std::endl;
}

void CompilationEngine::compileStatements() {
    outputStream << "<statements>" << std::endl;
    while (isStatement()) {
        if (tokenizer.keyWord() == KW_LET) {
            compileLet();
        }
        else if (tokenizer.keyWord() == KW_IF) {
            compileIf();
        }
        else if (tokenizer.keyWord() == KW_WHILE) {
            compileWhile();
        } 
        else if (tokenizer.keyWord() == KW_DO) {
            compileDo();
        }
        else {
            compileReturn();
        }
    }
    outputStream << "</statements>" << std::endl;
}

void CompilationEngine::compileLet() {
    outputStream << "<letStatement>" << std::endl;
    writeKeyWord(); // let
    std::string varName = tokenizer.identifier();
    if (classSymbolTable.exists(varName)) {
        writeVar(classSymbolTable.typeOf(varName), classSymbolTable.kindOf(varName), false, true);
    }
    else if (subroutineSymbolTable.exists(varName)) {
        writeVar(subroutineSymbolTable.typeOf(varName), subroutineSymbolTable.kindOf(varName), false, false);
    }
    if (tokenizer.tokenType() == SYMBOL && tokenizer.symbol() == '[') {
        writeSymbol(); // [
        compileExpression();
        writeSymbol(); // ]
    }
    writeSymbol(); // =
    compileExpression();
    writeSymbol(); // ;
    outputStream << "</letStatement>" << std::endl;
}

void CompilationEngine::compileIf() {
    outputStream << "<ifStatement>" << std::endl;
    writeKeyWord(); // if
    writeSymbol(); // (
    compileExpression();
    writeSymbol(); // )
    writeSymbol(); // {
    compileStatements();
    writeSymbol(); // }
    if (tokenizer.tokenType() == KEYWORD && tokenizer.keyWord() == KW_ELSE) {
        writeKeyWord(); // else
        writeSymbol(); // {
        compileStatements();
        writeSymbol(); // }
    }    
    outputStream << "</ifStatement>" << std::endl;
}

void CompilationEngine::compileWhile() {
    outputStream << "<whileStatement>" << std::endl;
    writeKeyWord(); // while
    writeSymbol(); // (
    compileExpression();
    writeSymbol(); // )
    writeSymbol(); // {
    compileStatements();
    writeSymbol(); // } 
    outputStream << "</whileStatement>" << std::endl;
}

void CompilationEngine::compileDo() {
    outputStream << "<doStatement>" << std::endl;
    writeKeyWord(); // do
    compileSubroutineCall();
    writeSymbol(); // ;    
    outputStream << "</doStatement>" << std::endl;
}

void CompilationEngine::compileReturn() {
    outputStream << "<returnStatement>" << std::endl;
    writeKeyWord(); // return
    if (tokenizer.tokenType() != SYMBOL || tokenizer.symbol() != ';') {
        compileExpression();
    }
    writeSymbol(); // ;
    outputStream << "</returnStatement>" << std::endl;
}

void CompilationEngine::compileSubroutineCall() {
    std::string name = tokenizer.identifier();
    tokenizer.advance();
    if (tokenizer.tokenType() == SYMBOL && tokenizer.symbol() == '.') { // (className | varName).subroutineName
        if (classSymbolTable.exists(name) || subroutineSymbolTable.exists(name)) {
            outputStream << "<identifier category=\"" << kindToStr(kindOf(name)) << "\" index=\"" << indexOf(name) << "\" usage=\"used\">" << name << "</identifier>" << std::endl;
        }
        else {
            outputStream << "<identifier category=\"class\"" << " index=\"" << indexOf(name) << "\" usage=\"used\">" << name << "</identifier>" << std::endl;
        }
        writeSymbol(); // .
        outputStream << "<identifier category=\"subroutine\" index=\"-1\" usage=\"used\">" << tokenizer.identifier() << "</identifier>" << std::endl;
        tokenizer.advance();
    }
    else {
        outputStream << "<identifier category=\"subroutine\" index=\"-1\" usage=\"used\">" << name << "</identifier>" << std::endl;
    }
    writeSymbol(); // (
    compileExpressionList();
    writeSymbol(); // )
}

void CompilationEngine::compileExpression() {
    outputStream << "<expression>" << std::endl;
    compileTerm();
    while (tokenizer.tokenType() == SYMBOL && isOp()) {
        writeSymbol(); // op
        compileTerm();
    }
    outputStream << "</expression>" << std::endl;
}

void CompilationEngine::compileTerm() {
    outputStream << "<term>" << std::endl;
    TokenType tt = tokenizer.tokenType();
    if (tt == INT_CONST) {
        writeIntConst();
    }
    else if (tt == STRING_CONST) {
        writeStrConst();
    }
    else if (isKeyWordConstant()) {
        writeKeyWord();
    }
    else if (tt == IDENTIFIER) {
        std::string name = tokenizer.identifier();
        tokenizer.advance();
        // subroutineName(expressionList)
        if (tokenizer.tokenType() == SYMBOL && tokenizer.symbol() == '(') {
            outputStream << "<identifier category=\"subroutine\" index=\"-1\" usage=\"used\">" << name << "</identifier>" << std::endl;
            writeSymbol();
            compileExpressionList();
            writeSymbol();
        }
        // (className|varName).subroutineName(expressionList)
        else if (tokenizer.tokenType() == SYMBOL && tokenizer.symbol() == '.') {
            if (classSymbolTable.exists(name) || subroutineSymbolTable.exists(name)) {
                outputStream << "<identifier category=\"" << kindToStr(kindOf(name)) << "\" index=\"" << indexOf(name) << "\" usage=\"used\">" << name << "</identifier>" << std::endl;
            }
            else {
                outputStream << "<identifier category=\"class\"" << " index=\"" << indexOf(name) << "\" usage=\"used\">" << name << "</identifier>" << std::endl;
            }
            writeSymbol(); // .
            outputStream << "<identifier category=\"subroutine\" index=\"-1\" usage=\"used\">" << tokenizer.identifier() << "</identifier>" << std::endl;
            tokenizer.advance();
            writeSymbol();
            compileExpressionList();
            writeSymbol();
        }
        // varName[expression]
        else if (tokenizer.tokenType() == SYMBOL && tokenizer.symbol() == '[') {
            outputStream << "<identifier category=\"" << kindToStr(kindOf(name)) << "\" index=\"" << indexOf(name) << "\" usage=\"used\">" << name << "</identifier>" << std::endl;
            writeSymbol(); // [
            compileExpression(); 
            writeSymbol(); // ]
        }
        else {
            outputStream << "<identifier category=\"" << kindToStr(kindOf(name)) << "\" index=\"" << indexOf(name) << "\" usage=\"used\">" << name << "</identifier>" << std::endl;
        }
    }
    else if (tt == SYMBOL && tokenizer.symbol() == '(') {
        // (expression)
        writeSymbol();
        compileExpression();
        writeSymbol();
    }
    else if (tt == SYMBOL && (tokenizer.symbol() == '-' || tokenizer.symbol() == '~')) {
        // unaryOp term
        writeSymbol();
        compileTerm();
    }
    outputStream << "</term>" << std::endl;
}

void CompilationEngine::compileExpressionList() {
    outputStream << "<expressionList>" << std::endl;
    if (tokenizer.tokenType() != SYMBOL || tokenizer.symbol() != ')') {
        compileExpression();
        while (tokenizer.tokenType() == SYMBOL && tokenizer.symbol() == ',') {
            writeSymbol(); // ,
            compileExpression();
        }
    }
    outputStream << "</expressionList>" << std::endl;
}

void CompilationEngine::writeKeyWord() {
    if (tokenizer.tokenType() != KEYWORD) {
        std::cerr << "Error: expected KEYWORD." << std::endl;
    }
    outputStream << "<keyword>" << keyWordToStr.at(tokenizer.keyWord()) << "</keyword>" << std::endl;
    tokenizer.advance();
}

void CompilationEngine::writeSymbol() {
    if (tokenizer.tokenType() != SYMBOL) {
        std::cerr << "Error: expected SYMBOL but got " << tokenizer.currentToken << std::endl;
    }
    std::string symbol{tokenizer.symbol()};
    if (symbol == "<") {
        symbol = "&lt;";
    }
    else if (symbol == ">") {
        symbol = "&gt;";
    }
    else if (symbol == "&") {
        symbol = "&amp;";
    }
    else if (symbol == "\"") {
        symbol = "&quot;";
    } 
    outputStream << "<symbol>" << symbol << "</symbol>" << std::endl;
    tokenizer.advance();
}

void CompilationEngine::writeIntConst() {
    if (tokenizer.tokenType() != INT_CONST) {
        std::cerr << "Error: expected INT_CONST." << std::endl;
    }
    outputStream << "<integerConstant>" << tokenizer.intVal() << "</integerConstant>" << std::endl;
    tokenizer.advance();
}

void CompilationEngine::writeStrConst() {
    if (tokenizer.tokenType() != STRING_CONST) {
        std::cerr << "Error: expected STR_CONST." << std::endl;
    }
    outputStream << "<stringConstant>" << tokenizer.stringVal() << "</stringConstant>" << std::endl;
    tokenizer.advance();
}

void CompilationEngine::writeVar(std::string type, Kind kind, bool isDeclaration, bool isClass) {
    if (tokenizer.tokenType() != IDENTIFIER) {
        std::cerr << "Error: expected IDENTIFIER." << std::endl;
    }
    std::string name = tokenizer.identifier();
    if (isDeclaration) isClass ? classSymbolTable.define(name, type, kind) : subroutineSymbolTable.define(name, type, kind);
    int index = indexOf(name);
    std::string declarationString = isDeclaration ? "declared" : "used";
    outputStream << "<identifier category=\"" << kindToStr(kind) << "\" index=\"" << index << "\" usage=\"" << declarationString << "\">" << name << "</identifier>" << std::endl;
    tokenizer.advance();
}

std::string CompilationEngine::kindToStr(Kind kind) {
    switch (kind) {
        case STATIC: return "static";
        case FIELD: return "field";
        case ARG: return "arg";
        case VAR: return "var";
        case NONE: return "-1";
        default: return "-1";
    }
}

void CompilationEngine::writeType() {
    if (!isType()) {
        std::cerr << "Error: expected type." << std::endl;
    }
    if (tokenizer.tokenType() == KEYWORD) {
        writeKeyWord();
    }
    else {
        outputStream << "<identifier category=\"class\" index=\"-1\" usage=\"used\">" << tokenizer.identifier() << "</identifier>" << std::endl;
        tokenizer.advance();
    }
}

bool CompilationEngine::isType() {
    if (tokenizer.tokenType() == KEYWORD) {
        return (tokenizer.keyWord() == KW_INT || tokenizer.keyWord() == KW_CHAR || tokenizer.keyWord() == KW_BOOLEAN);
    }
    return tokenizer.tokenType() == IDENTIFIER;
}

bool CompilationEngine::isStatement() {
    if (tokenizer.tokenType() != KEYWORD) return false;
    KeyWord keyword = tokenizer.keyWord();
    return (keyword == KW_LET || keyword == KW_IF || keyword == KW_WHILE || keyword == KW_DO || keyword == KW_RETURN);
}

bool CompilationEngine::isKeyWordConstant() {
    if (tokenizer.tokenType() != KEYWORD) return false;
    KeyWord kw = tokenizer.keyWord();
    return (kw == KW_TRUE || kw == KW_FALSE || kw == KW_NULL || kw == KW_THIS);
}

bool CompilationEngine::isOp() {
    static const std::vector<char> ops = {'+', '-', '*', '/', '&', '|', '<', '>', '='};
    return (std::find(ops.begin(), ops.end(), tokenizer.symbol()) != ops.end());
}

bool CompilationEngine::isUnaryOp() {
    return (isOp() && (tokenizer.symbol() == '-' || tokenizer.symbol() == '~')); 
}

bool CompilationEngine::isTerm() {
    TokenType tt = tokenizer.tokenType();
    return (tt == INT_CONST || tt == STRING_CONST || isKeyWordConstant() || tt == IDENTIFIER || isUnaryOp());
}

Kind CompilationEngine::kindOf(std::string name) {
    if (classSymbolTable.exists(name)) {
        return classSymbolTable.kindOf(name);
    }
    else if (subroutineSymbolTable.exists(name)) {
        return subroutineSymbolTable.kindOf(name);
    }
    return NONE;
}

int CompilationEngine::indexOf(std::string name) {
    if (classSymbolTable.exists(name)) {
        return classSymbolTable.indexOf(name);
    }
    else if (subroutineSymbolTable.exists(name)) {
        return subroutineSymbolTable.indexOf(name);
    }
    return -1;
}

std::string CompilationEngine::typeOf(std::string name) {
    if (classSymbolTable.exists(name)) {
        return classSymbolTable.typeOf(name);
    }
    else if (subroutineSymbolTable.exists(name)) {
        return subroutineSymbolTable.typeOf(name);
    }
    return std::string();
}
