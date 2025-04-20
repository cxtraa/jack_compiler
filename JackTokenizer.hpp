#pragma once

#include "Enums.hpp"
#include <string>
#include <fstream>
#include <filesystem>

class JackTokenizer {
public:
    JackTokenizer(std::filesystem::path inputFile);
    bool hasMoreTokens();
    void advance();
    TokenType tokenType();
    KeyWord keyWord();
    char symbol();
    std::string type();
    std::string identifier();
    int intVal();
    std::string stringVal();
    std::string currentToken;
private:
    char getChar();
    void advanceChar();
    bool isSymbol(char c);
    std::ifstream input;
    char currentChar;
    void skipWhitespaceAndComments();
};