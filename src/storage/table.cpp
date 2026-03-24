#include "storage/table.h"
#include <stdexcept>
#include <ctime>

Table::Table(const Schema& schema) : schema(schema) {}

void Table::insert(const Row& row) {
    if (row.get_values().size() != schema.column_count()) {
        throw std::runtime_error("Column count mismatch");
    }

    rows.push_back(row);
}

std::vector<Row> Table::get_all_rows() const {
    std::vector<Row> valid_rows;
    long long current_time = std::time(nullptr);

    for (const auto& row : rows) {
        if (row.get_expiration_time() > current_time) {
            valid_rows.push_back(row);
        }
    }

    return valid_rows;
}

const Schema& Table::get_schema() const {
    return schema;
}