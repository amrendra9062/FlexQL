#include "parser/parser.h"
#include <stdexcept>
#include <iostream>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), pos(0) {}

Token Parser::peek() {
    if (pos >= tokens.size()) return {TokenType::END_OF_FILE, ""};
    return tokens[pos];
}

Token Parser::get() {
    if (pos >= tokens.size()) return {TokenType::END_OF_FILE, ""};
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
    if (t.type == TokenType::DELETE) return parseDelete();

    throw std::runtime_error("Unknown query type: " + t.value);
}

Query Parser::parseSelect() {
    Query query;
    query.type = QueryType::SELECT;
    get(); // consume SELECT

    // ------------------ COLUMNS ------------------
    if (match(TokenType::STAR)) {
        query.selectQuery.select_all = true;
    } else {
        while (true) {
            std::string colName = get().value;
            // Handle table.column syntax
            if (match(TokenType::DOT)) {
                colName += "." + get().value;
            }
            query.selectQuery.columns.push_back(colName);
            if (!match(TokenType::COMMA)) break;
        }
    }

    // ------------------ FROM ------------------
    if (!match(TokenType::FROM)) throw std::runtime_error("Expected FROM");
    query.selectQuery.table = get().value;

    // ------------------ JOIN ------------------
    if (match(TokenType::INNER)) {
        if (!match(TokenType::JOIN)) throw std::runtime_error("Expected JOIN");
        query.selectQuery.has_join = true;
        query.selectQuery.join_table = get().value;
        
        if (!match(TokenType::ON)) throw std::runtime_error("Expected ON");
        
        std::string left_table = get().value;
        match(TokenType::DOT);
        std::string left_col = get().value;
        
        match(TokenType::EQUAL);
        
        std::string right_table = get().value;
        match(TokenType::DOT);
        std::string right_col = get().value;
        
        query.selectQuery.join_condition = {left_table, left_col, right_table, right_col};
    }

    // ------------------ WHERE ------------------
    if (match(TokenType::WHERE)) {
        query.selectQuery.where = parseWhere();
        query.selectQuery.has_where = true;
    }

    // ------------------ ORDER BY ------------------
    if (match(TokenType::ORDER)) {
        if (!match(TokenType::BY)) throw std::runtime_error("Expected BY after ORDER");
        
        query.selectQuery.has_order_by = true;
        
        std::string order_col = get().value;
        if (match(TokenType::DOT)) {
            order_col += "." + get().value;
        }
        query.selectQuery.order_by.column = order_col;
        
        if (match(TokenType::DESC)) {
            query.selectQuery.order_by.descending = true;
        } else {
            match(TokenType::ASC); // ASC is optional
            query.selectQuery.order_by.descending = false;
        }
    }

    // Skip optional semicolon
    match(TokenType::SEMICOLON);
    return query;
}

Condition Parser::parseWhere() {
    Condition cond;
    
    cond.column = get().value;
    if (match(TokenType::DOT)) {
        cond.column += "." + get().value;
    }

    Token op = get();
    if (op.type == TokenType::EQUAL || op.type == TokenType::GREATER || 
        op.type == TokenType::LESS || op.type == TokenType::GREATER_EQUAL || 
        op.type == TokenType::LESS_EQUAL) {
        cond.op = op.value;
    } else {
        throw std::runtime_error("Invalid operator in WHERE: " + op.value);
    }

    cond.value = get().value;
    return cond;
}

Query Parser::parseInsert() {
    Query query;
    query.type = QueryType::INSERT;
    get(); // INSERT
    match(TokenType::INTO);
    query.insertQuery.table = get().value;
    match(TokenType::VALUES);

    // Loop to support batch inserts: VALUES (1), (2), (3)
    while (match(TokenType::LPAREN)) {
        std::vector<std::string> row_values;
        while (true) {
            row_values.push_back(get().value);
            if (!match(TokenType::COMMA)) break;
        }
        match(TokenType::RPAREN);
        query.insertQuery.rows.push_back(row_values);
        
        if (!match(TokenType::COMMA)) break; // If no comma, we are done
    }

    match(TokenType::SEMICOLON);
    return query;
}

Query Parser::parseCreate() {
    Query query;
    query.type = QueryType::CREATE;
    get(); // CREATE
    match(TokenType::TABLE);

    // Support CREATE TABLE IF NOT EXISTS
    if (peek().value == "IF") {
        get(); // IF
        if (get().value == "NOT" && get().value == "EXISTS") {
            query.createQuery.if_not_exists = true;
        }
    }

    query.createQuery.table = get().value;
    match(TokenType::LPAREN);

    while (true) {
        std::string col = get().value;
        std::string type = get().value;
        
        // Skip size declarations like VARCHAR(64)
        if (match(TokenType::LPAREN)) {
            get(); // size number
            match(TokenType::RPAREN);
        }
        
        query.createQuery.columns.push_back({col, type});
        if (!match(TokenType::COMMA)) break;
    }
    match(TokenType::RPAREN);
    match(TokenType::SEMICOLON);
    return query;
}

Query Parser::parseDelete() {
    Query query;
    query.type = QueryType::DELETE;
    get(); // DELETE
    match(TokenType::FROM);
    
    query.deleteQuery.table = get().value;
    
    if (match(TokenType::WHERE)) {
        query.deleteQuery.has_where = true;
        query.deleteQuery.where = parseWhere();
    }
    
    match(TokenType::SEMICOLON);
    return query;
}