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
    std::string op;     // "=" for now
    std::string value;
};
struct SelectQuery {
    std::vector<std::string> columns;
    std::string table;
    bool select_all = false;
    Condition where;
    bool has_where = false;
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
