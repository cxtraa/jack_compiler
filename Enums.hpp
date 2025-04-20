#pragma once

#include <unordered_map>
#include <string>

enum TokenType {
    KEYWORD,
    SYMBOL,
    IDENTIFIER,
    INT_CONST,
    STRING_CONST
};

enum KeyWord {
    KW_CLASS,
    KW_METHOD,
    KW_FUNCTION,
    KW_CONSTRUCTOR,
    KW_INT,
    KW_BOOLEAN,
    KW_CHAR,
    KW_VOID,
    KW_VAR,
    KW_STATIC,
    KW_FIELD,
    KW_LET,
    KW_DO,
    KW_IF,
    KW_ELSE,
    KW_WHILE,
    KW_RETURN,
    KW_TRUE,
    KW_FALSE,
    KW_NULL,
    KW_THIS
};

enum Kind {
    STATIC,
    FIELD,
    ARG,
    VAR,
    NONE
};