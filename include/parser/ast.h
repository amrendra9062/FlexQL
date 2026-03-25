#pragma once
#include <string>
#include <vector>

enum class QueryType {
    SELECT,
    INSERT,
    CREATE,
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
};

struct InsertQuery {
    std::string table;
    std::vector<std::string> values;
};

struct CreateQuery {
    std::string table;
    std::vector<std::pair<std::string, std::string>> columns;
};

struct Query {
    QueryType type;

    SelectQuery selectQuery;
    InsertQuery insertQuery;
    CreateQuery createQuery;
};