#pragma once
#include "table.h"
#include <unordered_map>
#include <string>

class Database {
private:
    std::unordered_map<std::string, Table> tables;

public:
    void create_table(const Schema& schema);

    Table& get_table(const std::string& name);

    bool has_table(const std::string& name) const;
};