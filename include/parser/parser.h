#pragma once
#include "parser/tokenizer.h"
#include "parser/ast.h"

class Parser {
public:
    Parser(const std::vector<Token>& tokens);

    Query parse();

private:
    std::vector<Token> tokens;
    size_t pos; // Changed from int to size_t to prevent compiler warnings

    Token peek();
    Token get();
    bool match(TokenType type);

    Query parseSelect();
    Query parseInsert();
    Query parseCreate();
    
    // NEW: Added to support the DELETE parsing logic
    Query parseDelete(); 

    Condition parseWhere();
};