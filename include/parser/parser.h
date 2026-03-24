#pragma once
#include "tokenizer.h"
#include "ast.h"

class Parser {
public:
    Parser(const std::vector<Token>& tokens);

    Query parse();

private:
    std::vector<Token> tokens;
    int pos;

    Token peek();
    Token get();
    bool match(TokenType type);

    Query parseSelect();
    Query parseInsert();
    Query parseCreate();

    Condition parseWhere();
};