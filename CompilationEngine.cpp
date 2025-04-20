#include "CompilationEngine.hpp"
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <algorithm>

CompilationEngine::CompilationEngine(std::filesystem::path inputPath, std::filesystem::path outputPath) : tokenizer(inputPath), classSymbolTable(), subroutineSymbolTable(), labelNumber(0), vmWriter(outputPath) {
    currentClass = "Main";
    inputStream = std::ifstream(inputPath);
    if (!inputStream) throw std::runtime_error("CompilationEngine: the input file could not be read from.");

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
    writeKeyWord(); // class
    currentClass = tokenizer.identifier();
    writeIdentifier(); // className
    writeSymbol(); // {
    while (tokenizer.tokenType() == KEYWORD && (tokenizer.keyWord() == KW_STATIC || tokenizer.keyWord() == KW_FIELD)) {
        compileClassVarDec();
    }
    while (tokenizer.tokenType() == KEYWORD && (tokenizer.keyWord() == KW_CONSTRUCTOR || tokenizer.keyWord() == KW_FUNCTION || tokenizer.keyWord() == KW_METHOD)) {
        compileSubroutineDec();
    }
    writeSymbol(); // }
}

void CompilationEngine::compileClassVarDec() {
    Kind kind = tokenizer.keyWord() == KW_STATIC ? STATIC : FIELD;
    writeKeyWord(); // static | field
    std::string type = tokenizer.type();
    writeType(); // type
    classSymbolTable.define(tokenizer.identifier(), type, kind);
    writeIdentifier();
    while (tokenizer.tokenType() == SYMBOL && tokenizer.symbol() == ',') {
        writeSymbol(); // ,
        classSymbolTable.define(tokenizer.identifier(), type, kind);
        writeIdentifier(); // varName
    }
    writeSymbol(); // ;
}

void CompilationEngine::compileSubroutineDec() {
    subroutineSymbolTable.reset();
    KeyWord functionType = tokenizer.keyWord();
    if (functionType == KW_METHOD) {
        subroutineSymbolTable.define("this", currentClass, ARG);
    }
    writeKeyWord(); // constructor | function | method
    writeType(); // void | type
    std::string subroutineName = currentClass + "." + tokenizer.identifier();
    writeIdentifier(); // subroutineName
    writeSymbol(); // '('
    int numParameters = compileParameterList();
    writeSymbol(); // ')'

    // SUBROUTINE BODY
    writeSymbol(); // {
    while (tokenizer.tokenType() == KEYWORD && tokenizer.keyWord() == KW_VAR) {
        compileVarDec();
    }
    int nLocalVars = subroutineSymbolTable.varCount(VAR);
    vmWriter.writeFunction(subroutineName, nLocalVars);
    if (functionType == KW_METHOD) {
        vmWriter.writePush("argument", 0);
        vmWriter.writePop("pointer", 0);
    }
    else if (functionType == KW_CONSTRUCTOR) {
        int nFieldVars = classSymbolTable.varCount(FIELD);
        vmWriter.writePush("constant", nFieldVars);
        vmWriter.writeCall("Memory.alloc", 1);
        vmWriter.writePop("pointer", 0);
    }
    compileStatements();
    if (functionType == KW_CONSTRUCTOR) {
        vmWriter.writePush("pointer", 0);
    }
    writeSymbol(); // }
}

int CompilationEngine::compileParameterList() {
    int numParameters = 0;
    if (isType()) { // (type varName)
        numParameters++;
        std::string type = tokenizer.type();
        writeType(); // type
        subroutineSymbolTable.define(tokenizer.identifier(), type, ARG);
        writeIdentifier();
    }
    while (tokenizer.tokenType() == SYMBOL && tokenizer.symbol() == ',') { // (',' type varName)*
        numParameters++;
        writeSymbol(); // ,
        std::string type = tokenizer.type();
        writeType(); // type
        subroutineSymbolTable.define(tokenizer.identifier(), type, ARG);
        writeIdentifier();
    }
    return numParameters;
}

void CompilationEngine::compileVarDec() {
    writeKeyWord(); // var

    std::string type = tokenizer.type();
    writeType(); // type
    subroutineSymbolTable.define(tokenizer.identifier(), type, VAR);
    writeIdentifier(); // varName
    while (tokenizer.tokenType() == SYMBOL && tokenizer.symbol() == ',') {
        writeSymbol(); // ,
        subroutineSymbolTable.define(tokenizer.identifier(), type, VAR);
        writeIdentifier(); // varName
    }
    writeSymbol(); // ;
}

void CompilationEngine::compileStatements() {
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
        else if (tokenizer.keyWord() == KW_RETURN) {
            compileReturn();
        }
    }
}

void CompilationEngine::compileLet() {
    writeKeyWord(); // let
    bool isArrayAccess = false;
    std::string name = tokenizer.identifier();
    writeIdentifier(); // go forward
    if (!classSymbolTable.exists(name) && !subroutineSymbolTable.exists(name)) {
        throw std::runtime_error("Attempted to assign to an undefined variable.");
    }
    if (tokenizer.tokenType() == SYMBOL && tokenizer.symbol() == '[') {
        isArrayAccess = true;
        vmWriter.writePush(kindToStr(kindOf(name)), indexOf(name));
        writeSymbol(); // [
        compileExpression();
        writeSymbol(); // ]
        vmWriter.writeArithmetic("add");
    }
    writeSymbol(); // =
    compileExpression();
    if (isArrayAccess) {
        vmWriter.writePop("temp", 0);
        vmWriter.writePop("pointer", 1);
        vmWriter.writePush("temp", 0);
        vmWriter.writePop("that", 0);
    }
    else {
        vmWriter.writePop(kindToStr(kindOf(name)), indexOf(name));
    }
    writeSymbol(); // ;
}

void CompilationEngine::compileIf() {
    writeKeyWord(); // if
    writeSymbol(); // (
    compileExpression();
    writeSymbol(); // )
    writeSymbol(); // {
    vmWriter.writeArithmetic("not");
    std::string L1 = "L" + std::to_string(2*labelNumber);
    std::string L2 = "L" + std::to_string(2*labelNumber + 1);
    labelNumber++;
    vmWriter.writeIf(L1);
    compileStatements();
    vmWriter.writeGoTo(L2);
    writeSymbol(); // }
    vmWriter.writeLabel(L1);
    if (tokenizer.tokenType() == KEYWORD && tokenizer.keyWord() == KW_ELSE) {
        writeKeyWord(); // else
        writeSymbol(); // {
        compileStatements();
        writeSymbol(); // }
    }    
    vmWriter.writeLabel(L2);
}

void CompilationEngine::compileWhile() {
    std::string L1 = "L" + std::to_string(2*labelNumber);
    std::string L2 = "L" + std::to_string(2*labelNumber + 1);
    labelNumber++;
    vmWriter.writeLabel(L1);
    writeKeyWord(); // while
    writeSymbol(); // (
    compileExpression();
    writeSymbol(); // )
    vmWriter.writeArithmetic("not");
    vmWriter.writeIf(L2);
    writeSymbol(); // {
    compileStatements();
    vmWriter.writeGoTo(L1);
    vmWriter.writeLabel(L2);
    writeSymbol(); // } 
}

void CompilationEngine::compileDo() {
    writeKeyWord(); // do
    compileSubroutineCall();
    writeSymbol(); // ;
    vmWriter.writePop("temp", 0); // pop off returned value    
}

void CompilationEngine::compileReturn() {
    tokenizer.advance(); // return
    if (tokenizer.tokenType() != SYMBOL || tokenizer.symbol() != ';') {
        compileExpression();
    }
    else {
        vmWriter.writePush("constant", 0);
    }
    writeSymbol(); // ;
    vmWriter.writeReturn();
}

void CompilationEngine::compileSubroutineCall() {
    std::string name = tokenizer.identifier();
    writeIdentifier(); // name
    // (className | varName).subroutineName
    if (tokenizer.tokenType() == SYMBOL && tokenizer.symbol() == '.') { 
        compileClassVarSubroutineCall(name);
    }
    // subroutineName(expressionList)
    else {
        compileCurrentObjectSubroutineCall(name);
    }
}

void CompilationEngine::compileExpression() {
    compileTerm();
    while (tokenizer.tokenType() == SYMBOL && isOp()) {
        std::string op = binaryOp();
        writeSymbol();
        compileTerm();
        vmWriter.writeArithmetic(op);
    }
}

void CompilationEngine::compileTerm() {
    TokenType tt = tokenizer.tokenType();
    if (tt == INT_CONST) {
        writeIntConst();
    }
    else if (tt == STRING_CONST) {
        writeStrConst();
    }
    else if (isKeyWordConstant()) {
        writeKeyWordConst();
    }
    else if (tt == IDENTIFIER) {
        std::string name = tokenizer.identifier();
        tokenizer.advance();
        // subroutineName(expressionList)
        if (tokenizer.tokenType() == SYMBOL && tokenizer.symbol() == '(') {
            compileCurrentObjectSubroutineCall(name);
        }
        // (className|varName).subroutineName(expressionList)
        else if (tokenizer.tokenType() == SYMBOL && tokenizer.symbol() == '.') {
            compileClassVarSubroutineCall(name);
        }
        // varName[expression]
        else if (tokenizer.tokenType() == SYMBOL && tokenizer.symbol() == '[') {
            vmWriter.writePush(kindToStr(kindOf(name)), indexOf(name));
            writeSymbol(); // [
            compileExpression(); 
            writeSymbol(); // ]
            vmWriter.writeArithmetic("add");
            vmWriter.writePop("pointer", 1);
            vmWriter.writePush("that", 0);
        }
        else {
            vmWriter.writePush(kindToStr(kindOf(name)), indexOf(name));
        }
    }
    else if (tt == SYMBOL && tokenizer.symbol() == '(') {
        // (expression)
        writeSymbol(); // (
        compileExpression();
        writeSymbol(); // )
    }
    else if (tt == SYMBOL && (tokenizer.symbol() == '-' || tokenizer.symbol() == '~')) {
        // unaryOp term
        std::string op = unaryOp();
        writeSymbol(); // op
        compileTerm();
        vmWriter.writeArithmetic(op);
    }
}

void CompilationEngine::compileCurrentObjectSubroutineCall(std::string name) {
    vmWriter.writePush("pointer", 0);
    writeSymbol(); // (
    int numExpressions = compileExpressionList();
    writeSymbol(); // )
    vmWriter.writeCall(currentClass + "." + name, numExpressions + 1);
}

void CompilationEngine::compileClassVarSubroutineCall(std::string name) {
    bool isStatic = !(classSymbolTable.exists(name) || subroutineSymbolTable.exists(name)); // is this a call to static function?
    std::string functionName;
    if (!isStatic) {
        vmWriter.writePush(kindToStr(kindOf(name)), indexOf(name)); // push object to stack
        functionName += typeOf(name);
    }
    else {
        functionName += name;
    }
    writeSymbol(); // .
    functionName += "." + tokenizer.identifier();
    tokenizer.advance(); // subroutineName
    writeSymbol(); // (
    int numExpressions = compileExpressionList();
    writeSymbol(); // )
    vmWriter.writeCall(functionName, isStatic ? numExpressions : numExpressions + 1); // if not static, 'this' is an extra arg
}

int CompilationEngine::compileExpressionList() {
    int numExpressions = 0;
    if (tokenizer.tokenType() != SYMBOL || tokenizer.symbol() != ')') {
        compileExpression();
        numExpressions++;
        while (tokenizer.tokenType() == SYMBOL && tokenizer.symbol() == ',') {
            tokenizer.advance(); // ,
            compileExpression();
            numExpressions++;
        }
    }
    return numExpressions;
}

void CompilationEngine::writeKeyWord() {
    if (tokenizer.tokenType() != KEYWORD) {
        std::cerr << "Error at line " << tokenizer.getLineNumber() << ": expected KEYWORD but got " << tokenizer.currentToken << std::endl;
    }
    tokenizer.advance();
}

void CompilationEngine::writeType() {
    if (tokenizer.tokenType() != KEYWORD && tokenizer.tokenType() != IDENTIFIER) {
        std::cerr << "Error at line " << tokenizer.getLineNumber() << ": expected type but got " << tokenizer.currentToken << std::endl;
    }
    tokenizer.advance();
}

void CompilationEngine::writeSymbol() {
    if (tokenizer.tokenType() != SYMBOL) {
        std::cerr << "Error at line " << tokenizer.getLineNumber() << ": expected SYMBOL but got " << tokenizer.currentToken << std::endl;
        tokenizer.advance();
    }
    tokenizer.advance();
}

void CompilationEngine::writeIdentifier() {
    if (tokenizer.tokenType() != IDENTIFIER) {
        std::cerr << "Error at line " << tokenizer.getLineNumber() << ": expected identifier but got " << tokenizer.currentToken << std::endl;
    }
    tokenizer.advance();
}

void CompilationEngine::writeIntConst() {
    if (tokenizer.tokenType() != INT_CONST) {
        std::cerr << "Error at line " << tokenizer.getLineNumber() << ": expected INT_CONST but got " << tokenizer.currentToken << std::endl;
    }
    vmWriter.writePush("constant", tokenizer.intVal());
    tokenizer.advance();
}

void CompilationEngine::writeStrConst() {
    if (tokenizer.tokenType() != STRING_CONST) {
        std::cerr << "Error at line " << tokenizer.getLineNumber() << ": expected STR_CONST but got " << tokenizer.currentToken << std::endl;
    }
    std::string str = tokenizer.stringVal();
    vmWriter.writePush("constant", str.length());
    vmWriter.writeCall("String.new", 1);
    for (int i = 0; i < str.length(); i++) {
        int c = str[i];
        vmWriter.writePush("constant", c);
        vmWriter.writeCall("String.appendChar", 2);
    }
    tokenizer.advance();
}

std::string CompilationEngine::kindToStr(Kind kind) {
    switch (kind) {
        case STATIC: return "static";
        case FIELD: return "this";
        case ARG: return "argument";
        case VAR: return "local";
        case NONE: return "-1";
        default: return "-1";
    }
}

void CompilationEngine::writeKeyWordConst() {
    if (tokenizer.tokenType() != KEYWORD) {
        std::cerr << "Error at line " << tokenizer.getLineNumber() << ": expected KEYWORD_CONST but got " << tokenizer.currentToken << std::endl;
    }
    if (tokenizer.keyWord() == KW_TRUE) {
        vmWriter.writePush("constant", 0);
        vmWriter.writeArithmetic("not");
    }
    else if (tokenizer.keyWord() == KW_FALSE) {
        vmWriter.writePush("constant", 0);
    }
    else if (tokenizer.keyWord() == KW_NULL) {
        vmWriter.writePush("constant", 0);
    }
    else if (tokenizer.keyWord() == KW_THIS) {
        vmWriter.writePush("pointer", 0);
    }
    else {
        std::cerr << "Error at line " << tokenizer.getLineNumber() << ": expected KEYWORD_CONST but got " << tokenizer.currentToken << std::endl;
    }
    tokenizer.advance();
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

std::string CompilationEngine::binaryOp() {
    if (tokenizer.tokenType() != SYMBOL) {
        std::cerr << "Error at line " << tokenizer.getLineNumber() << ": expected binary operator but got " << tokenizer.currentToken << std::endl;
    }
    switch (tokenizer.symbol()) {
        case '+': return "add";
        case '-': return "sub";
        case '*': return "call Math.multiply 2";
        case '/': return "call Math.divide 2";
        case '&': return "and";
        case '|': return "or";
        case '<': return "lt";
        case '>': return "gt";
        case '=': return "eq";
        default : std::cerr << "Error at line " << tokenizer.getLineNumber() << ": expected binary operator but got " << tokenizer.currentToken << std::endl;
    }
    return "";
}

std::string CompilationEngine::unaryOp() {
    if (tokenizer.tokenType() != SYMBOL) {
        std::cerr << "Error at line " << tokenizer.getLineNumber() << ": expected unary operator but got " << tokenizer.currentToken << std::endl;
    }
    switch (tokenizer.symbol()) {
        case '-' : return "neg";
        case '~' : return "not";
        default : std::cerr << "Error at line " << tokenizer.getLineNumber() << ": expected unary operator but got " << tokenizer.currentToken << std::endl;
    }
    return "";
}

bool CompilationEngine::isTerm() {
    TokenType tt = tokenizer.tokenType();
    return (tt == INT_CONST || tt == STRING_CONST || isKeyWordConstant() || tt == IDENTIFIER || isUnaryOp());
}

Kind CompilationEngine::kindOf(std::string name) {
    if (subroutineSymbolTable.exists(name)) {
        return subroutineSymbolTable.kindOf(name);
    }
    else if (classSymbolTable.exists(name)) {
        return classSymbolTable.kindOf(name);
    }
    return NONE;
}

int CompilationEngine::indexOf(std::string name) {
    if (subroutineSymbolTable.exists(name)) {
        return subroutineSymbolTable.indexOf(name);
    }
    else if (classSymbolTable.exists(name)) {
        return classSymbolTable.indexOf(name);
    }
    return -1;
}

std::string CompilationEngine::typeOf(std::string name) {
    if (subroutineSymbolTable.exists(name)) {
        return subroutineSymbolTable.typeOf(name);
    }
    else if (classSymbolTable.exists(name)) {
        return classSymbolTable.typeOf(name);
    }
    return std::string();
}