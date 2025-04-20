#include "JackTokenizer.hpp"
#include "Enums.hpp"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <unordered_map>

JackTokenizer::JackTokenizer(std::filesystem::path inputPath) : lineNumber(0) {
    input = std::ifstream(inputPath);
    if (!input) {
        throw std::runtime_error("JackTokenizer: the requested file could not be opened.");
    }
    advanceChar();
}

bool JackTokenizer::hasMoreTokens() {
    skipWhitespaceAndComments();
    return (!input.eof());
}

int JackTokenizer::getLineNumber() {
    return lineNumber;
}

char JackTokenizer::getChar() {
    return input.peek();
}

void JackTokenizer::advanceChar() {
    currentChar = input.get();
    if (currentChar == '\n') lineNumber++;
}

bool JackTokenizer::isSymbol(char c) {
    static const std::string symbols = "{}()[].,;+-*/&|<>=~";
    return symbols.find(c) != std::string::npos;
}

void JackTokenizer::skipWhitespaceAndComments() {
    while (!input.eof()) {
        if (std::isspace(currentChar)) {
            advanceChar();
        }
        else if (currentChar == '/') {
            if (getChar() == '/') {
                advanceChar();
                advanceChar();
                while (!input.eof() && currentChar != '\n') {
                    advanceChar();
                }
            }
            else if (getChar() == '*') {
                advanceChar();
                advanceChar();
                while (!input.eof()) {
                    if (currentChar == '*' && getChar() == '/') {
                        advanceChar();
                        advanceChar();
                        break;
                    }
                    advanceChar();
                }
            }
            else {
                break;
            }
        }
        else {
            break;
        }
    }
}

void JackTokenizer::advance() {
    if (!hasMoreTokens()) return;
    currentToken.clear();
    if (currentChar == '"') {
        currentToken += currentChar;
        advanceChar();
        while (!input.eof() && currentChar != '"') {
            currentToken += currentChar;
            advanceChar();
        }
        currentToken += currentChar;
        advanceChar();
    }
    else if (isSymbol(currentChar)) {
        currentToken += currentChar;
        advanceChar();
    }
    else if (std::isalpha(currentChar) || currentChar == '_') {
        while (!input.eof() && (std::isalnum(currentChar) || currentChar == '_')) {
            currentToken += currentChar;
            advanceChar();
        }
    }
    else if (std::isdigit(currentChar)) {
        while (!input.eof() && std::isdigit(currentChar)) {
            currentToken += currentChar;
            advanceChar();
        }
    }
}

TokenType JackTokenizer::tokenType() {
    static const std::vector<std::string> keyWords = {
        "class", "method", "function", "constructor",
        "int", "boolean", "char", "void", "var", "static",
        "field", "let", "do", "if", "else", "while", "return",
        "true", "false", "null", "this"
    };

    if (std::isdigit(currentToken[0])) {
        return INT_CONST;
    }
    else if (isSymbol(currentToken[0])) {
        return SYMBOL;
    }
    else if (std::find(keyWords.begin(), keyWords.end(), currentToken) != keyWords.end()) {
        return KEYWORD;
    }
    else if (currentToken[0] == '"') {
        return STRING_CONST;
    }
    else if (std::isalpha(currentToken[0]) || currentToken[0] == '_') {
        return IDENTIFIER;
    }
    else {
        throw std::runtime_error("The token type was invalid.");
    }
}

KeyWord JackTokenizer::keyWord() {
    if (tokenType() != KEYWORD) {
        throw std::runtime_error("JackTokenizer: keyWord() was called when the current token is not a keyword!");
    }

    static const std::unordered_map<std::string, KeyWord> keyWordLookUp = {
        {"class", KW_CLASS},
        {"method", KW_METHOD},
        {"function", KW_FUNCTION},
        {"constructor", KW_CONSTRUCTOR},
        {"int", KW_INT},
        {"boolean", KW_BOOLEAN},
        {"char", KW_CHAR},
        {"void", KW_VOID},
        {"var", KW_VAR},
        {"static", KW_STATIC},
        {"field", KW_FIELD},
        {"let", KW_LET},
        {"do", KW_DO},
        {"if", KW_IF},
        {"else", KW_ELSE},
        {"while", KW_WHILE},
        {"return", KW_RETURN},
        {"true", KW_TRUE},
        {"false", KW_FALSE},
        {"null", KW_NULL},
        {"this", KW_THIS}
    };

    return keyWordLookUp.at(currentToken);
}

char JackTokenizer::symbol() {
    if (tokenType() != SYMBOL) {
        throw std::runtime_error("JackTokenizer: symbol() was called when the current token is not a symbol!");
    }
    return currentToken[0];
}

std::string JackTokenizer::type() {
    if (tokenType() == KEYWORD) {
        if (keyWord() == KW_INT) return "int";
        if (keyWord() == KW_CHAR) return "char";
        if (keyWord() == KW_BOOLEAN) return "bool";
    }
    else if (tokenType() == IDENTIFIER) {
        return identifier();
    }
    throw std::runtime_error("JackTokenizer: type() was called on token that cannot be a type!");
}

std::string JackTokenizer::identifier() {
    if (tokenType() != IDENTIFIER) {
        throw std::runtime_error("JackTokenizer: identifier() was called when the current token is not an identifier!");
    }
    return currentToken;
}

int JackTokenizer::intVal() {
    if (tokenType() != INT_CONST) {
        throw std::runtime_error("JackTokenizer: intVal() was called when the current token is not an integer constant!");
    }
    return std::stoi(currentToken);
}

std::string JackTokenizer::stringVal() {
    if (tokenType() != STRING_CONST) {
        throw std::runtime_error("JackTokenizer: identifier() was called when the current token is not an identifier!");
    }
    return currentToken.substr(1, currentToken.length() - 2); // remove double quotes
}