#include "parser/parser.h"
#include <stdexcept>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), pos(0) {}

Token Parser::peek() {
    return tokens[pos];
}

Token Parser::get() {
    return tokens[pos++];
}

bool Parser::match(TokenType type) {
    if (peek().type == type) {
        get();
        return true;
    }
    return false;
}
Query Parser::parse() {
    Token t = peek();

    if (t.type == TokenType::SELECT) return parseSelect();
    if (t.type == TokenType::INSERT) return parseInsert();
    if (t.type == TokenType::CREATE) return parseCreate();

    throw std::runtime_error("Unknown query type");
}
Query Parser::parseSelect() {
    Query query;
    query.type = QueryType::SELECT;

    get(); // consume SELECT

    // columns
    if (match(TokenType::STAR)) {
        query.selectQuery.select_all = true;
    } else {
        while (true) {
            Token col = get();
            query.selectQuery.columns.push_back(col.value);

            if (!match(TokenType::COMMA)) break;
        }
    }

    // FROM
    if (!match(TokenType::FROM)) {
        throw std::runtime_error("Expected FROM");
    }

    // table name
    query.selectQuery.table = get().value;

    // WHERE (optional)
    if (match(TokenType::WHERE)) {
        query.selectQuery.where = parseWhere();
        query.selectQuery.has_where = true;
    }

    return query;
}
Condition Parser::parseWhere() {
    Condition cond;

    cond.column = get().value;

    Token op = get();
    cond.op = op.value;

    Token val = get();
    cond.value = val.value;

    return cond;
}
Query Parser::parseInsert() {
    Query query;
    query.type = QueryType::INSERT;

    get(); // INSERT
    match(TokenType::INTO);

    query.insertQuery.table = get().value;

    match(TokenType::VALUES);
    match(TokenType::LPAREN);

    while (true) {
        query.insertQuery.values.push_back(get().value);
        if (!match(TokenType::COMMA)) break;
    }

    match(TokenType::RPAREN);

    return query;
}
Query Parser::parseCreate() {
    Query query;
    query.type = QueryType::CREATE;

    get(); // CREATE
    match(TokenType::TABLE);

    query.createQuery.table = get().value;

    match(TokenType::LPAREN);

    while (true) {
        std::string col = get().value;
        std::string type = get().value;

        query.createQuery.columns.push_back({col, type});

        if (!match(TokenType::COMMA)) break;
    }

    match(TokenType::RPAREN);

    return query;
}
