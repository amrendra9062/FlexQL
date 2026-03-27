#pragma once
#include <string>
#include <vector>

enum class TokenType {
    // Keywords
    SELECT, INSERT, CREATE, TABLE, FROM, WHERE, INTO, VALUES, INNER, JOIN, ON,
    DELETE, ORDER, BY, ASC, DESC,

    // Symbols
    STAR, COMMA, SEMICOLON, LPAREN, RPAREN, EQUAL,
    GREATER, LESS, GREATER_EQUAL, LESS_EQUAL, DOT,

    // Literals
    IDENTIFIER,
    NUMBER,
    STRING,

    END_OF_FILE,
    UNKNOWN
};

struct Token {
    TokenType type;
    std::string value;
};

class Tokenizer {
public:
    Tokenizer(const std::string& input);

    std::vector<Token> tokenize();

private:
    std::string input;
    size_t pos;

    char peek();
    char peek_next(); // New: Lookahead for 2-char operators like >=
    char get();
    void skipWhitespace();

    Token identifier();
    Token number();
    Token string();
};

std::string tokenTypeToString(TokenType type);