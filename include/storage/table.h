#pragma once
#include "storage/schema.h"
#include "storage/row.h"
#include "index/index.h"
#include <vector>
#include <string>

class Table {
private:
    Schema schema;
    std::vector<Row> rows;
    Index index;   // index on first column

public:
    Table(const Schema& schema);
    
    void insert(const Row& row);
    std::vector<Row> get_all_rows() const;
    const Schema& get_schema() const;
    std::vector<Row> get_rows_by_index(const std::string& key);
    
    // NEW: Needed for resetting tables in the benchmark
    int delete_all_rows(); 
};