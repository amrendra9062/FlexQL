#include <iostream>
#include "parser/tokenizer.h"
#include <cctype>

Tokenizer::Tokenizer(const std::string& input) : input(input), pos(0) {}

char Tokenizer::peek() {
    return pos < input.size() ? input[pos] : '\0';
}

char Tokenizer::get() {
    return pos < input.size() ? input[pos++] : '\0';
}

void Tokenizer::skipWhitespace() {
    while (isspace(peek())) get();
}

Token Tokenizer::identifier() {
    std::string value;

    while (isalnum(peek()) || peek() == '_') {
        value += get();
    }

    // Convert to uppercase for keyword matching
    std::string upper = value;
    for (auto &c : upper) c = toupper(c);

    if (upper == "SELECT") return {TokenType::SELECT, value};
    if (upper == "FROM") return {TokenType::FROM, value};
    if (upper == "WHERE") return {TokenType::WHERE, value};
    if (upper == "INSERT") return {TokenType::INSERT, value};
    if (upper == "INTO") return {TokenType::INTO, value};
    if (upper == "VALUES") return {TokenType::VALUES, value};
    if (upper == "CREATE") return {TokenType::CREATE, value};
    if (upper == "TABLE") return {TokenType::TABLE, value};
    if (upper == "INNER") return {TokenType::INNER, value};
    if (upper == "JOIN") return {TokenType::JOIN, value};
    if (upper == "ON") return {TokenType::ON, value};

    return {TokenType::IDENTIFIER, value};
}

Token Tokenizer::number() {
    std::string value;

    while (isdigit(peek())) {
        value += get();
    }

    return {TokenType::NUMBER, value};
}

Token Tokenizer::string() {
    std::string value;
    get(); // skip opening quote

    while (peek() != '\'' && peek() != '\0') {
        value += get();
    }

    get(); // skip closing quote
    return {TokenType::STRING, value};
}

std::vector<Token> Tokenizer::tokenize() {
    std::vector<Token> tokens;

    while (true) {
        skipWhitespace();
        char c = peek();

        if (c == '\0') {
            tokens.push_back({TokenType::END_OF_FILE, ""});
            break;
        }

        if (isalpha(c)) {
            tokens.push_back(identifier());
        }
        else if (isdigit(c)) {
            tokens.push_back(number());
        }
        else if (c == '\'') {
            tokens.push_back(string());
        }
        else {
            switch (c) {
                case '*': tokens.push_back({TokenType::STAR, "*"}); break;
                case ',': tokens.push_back({TokenType::COMMA, ","}); break;
                case ';': tokens.push_back({TokenType::SEMICOLON, ";"}); break;
                case '(': tokens.push_back({TokenType::LPAREN, "("}); break;
                case ')': tokens.push_back({TokenType::RPAREN, ")"}); break;
                case '=': tokens.push_back({TokenType::EQUAL, "="}); break;
                default: tokens.push_back({TokenType::UNKNOWN, std::string(1, c)});
            }
            get();
        }
    }

    return tokens;
}
std::string tokenTypeToString(TokenType type) {
    switch(type) {
        case TokenType::SELECT: return "SELECT";
        case TokenType::FROM: return "FROM";
        case TokenType::WHERE: return "WHERE";
        case TokenType::INSERT: return "INSERT";
        case TokenType::CREATE: return "CREATE";
        case TokenType::TABLE: return "TABLE";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::STRING: return "STRING";
        case TokenType::EQUAL: return "EQUAL";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::END_OF_FILE: return "EOF";
        default: return "UNKNOWN";
    }
}
// int main() {
//     std::string query = "SELECT name FROM student WHERE id = 1;";
//     Tokenizer tokenizer(query);

//     auto tokens = tokenizer.tokenize();
//     for (auto &t : tokens) {
//         std::cout << tokenTypeToString(t.type) 
//                   << " -> " << t.value << std::endl;
//     }

//     return 0;
// }