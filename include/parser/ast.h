#pragma once
#include <string>
#include <vector>

enum class QueryType {
    SELECT,
    INSERT,
    CREATE,
    DELETE,
    UNKNOWN
};

struct Condition {
    std::string column;
    std::string op;
    std::string value;
};

struct JoinCondition {
    std::string left_table;
    std::string left_column;
    std::string right_table;
    std::string right_column;
};

struct OrderBy {
    std::string column;
    bool descending = false;
};

struct SelectQuery {
    std::vector<std::string> columns;
    std::string table;

    bool select_all = false;

    // WHERE
    Condition where;
    bool has_where = false;

    // JOIN
    bool has_join = false;
    std::string join_table;
    JoinCondition join_condition;

    // ORDER BY
    bool has_order_by = false;
    OrderBy order_by;
};

struct InsertQuery {
    std::string table;
    // Supports batch inserts: VALUES (1, 'A'), (2, 'B')
    std::vector<std::vector<std::string>> rows;
};

struct CreateQuery {
    std::string table;
    std::vector<std::pair<std::string, std::string>> columns;
    bool if_not_exists = false;
};

struct DeleteQuery {
    std::string table;
    bool has_where = false;
    Condition where;
};

struct Query {
    QueryType type;

    SelectQuery selectQuery;
    InsertQuery insertQuery;
    CreateQuery createQuery;
    DeleteQuery deleteQuery;
};