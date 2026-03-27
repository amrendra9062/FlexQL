#include "parser/tokenizer.h"
#include <cctype>

Tokenizer::Tokenizer(const std::string& input) : input(input), pos(0) {}

char Tokenizer::peek() {
    return pos < input.size() ? input[pos] : '\0';
}

char Tokenizer::peek_next() {
    return pos + 1 < input.size() ? input[pos + 1] : '\0';
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
    if (upper == "DELETE") return {TokenType::DELETE, value};
    if (upper == "ORDER") return {TokenType::ORDER, value};
    if (upper == "BY") return {TokenType::BY, value};
    if (upper == "ASC") return {TokenType::ASC, value};
    if (upper == "DESC") return {TokenType::DESC, value};

    return {TokenType::IDENTIFIER, value};
}

Token Tokenizer::number() {
    std::string value;

    // Allow digits and decimals for floating point numbers
    while (isdigit(peek()) || peek() == '.') {
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
            // Handle multi-character operators first
            if (c == '>' && peek_next() == '=') {
                tokens.push_back({TokenType::GREATER_EQUAL, ">="});
                get(); get(); // consume both
                continue;
            }
            if (c == '<' && peek_next() == '=') {
                tokens.push_back({TokenType::LESS_EQUAL, "<="});
                get(); get(); // consume both
                continue;
            }

            // Single character symbols
            switch (c) {
                case '*': tokens.push_back({TokenType::STAR, "*"}); break;
                case ',': tokens.push_back({TokenType::COMMA, ","}); break;
                case ';': tokens.push_back({TokenType::SEMICOLON, ";"}); break;
                case '(': tokens.push_back({TokenType::LPAREN, "("}); break;
                case ')': tokens.push_back({TokenType::RPAREN, ")"}); break;
                case '=': tokens.push_back({TokenType::EQUAL, "="}); break;
                case '>': tokens.push_back({TokenType::GREATER, ">"}); break;
                case '<': tokens.push_back({TokenType::LESS, "<"}); break;
                case '.': tokens.push_back({TokenType::DOT, "."}); break;
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
        case TokenType::DELETE: return "DELETE";
        case TokenType::ORDER: return "ORDER";
        case TokenType::BY: return "BY";
        case TokenType::DESC: return "DESC";
        case TokenType::ASC: return "ASC";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::STRING: return "STRING";
        case TokenType::EQUAL: return "EQUAL";
        case TokenType::GREATER: return "GREATER";
        case TokenType::LESS: return "LESS";
        case TokenType::GREATER_EQUAL: return "GREATER_EQUAL";
        case TokenType::LESS_EQUAL: return "LESS_EQUAL";
        case TokenType::DOT: return "DOT";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::END_OF_FILE: return "EOF";
        default: return "UNKNOWN";
    }
}