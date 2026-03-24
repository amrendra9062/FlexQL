#pragma once
#include "schema.h"
#include "row.h"
#include <vector>
#include <string>

class Table {
private:
    Schema schema;
    std::vector<Row> rows;

public:
    Table(const Schema& schema);

    void insert(const Row& row);

    std::vector<Row> get_all_rows() const;

    const Schema& get_schema() const;
};