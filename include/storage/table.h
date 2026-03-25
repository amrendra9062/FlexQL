#pragma once
#include "schema.h"
#include "row.h"
#include <vector>
#include <string>
#include "index/index.h"
class Table {
private:
    Schema schema;
    std::vector<Row> rows;
    HashIndex index;   // index on first column
public:
    Table(const Schema& schema);
    void insert(const Row& row);
    std::vector<Row> get_all_rows() const;
    const Schema& get_schema() const;
    std::vector<Row> get_rows_by_index(const std::string& key);
};