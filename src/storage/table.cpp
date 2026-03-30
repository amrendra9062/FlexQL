#include "storage/table.h"
#include <stdexcept>
#include <ctime>

Table::Table(const Schema& schema) : schema(schema) {}

void Table::insert(const Row& row) {
    // 1. Validate the row matches the table schema
    if (row.get_values().size() != schema.column_count()) {
        throw std::runtime_error("Column count mismatch");
    }

    // 2. Insert the row into physical memory
    rows.push_back(row);

    // NOTE: We intentionally removed the index.insert() call here!
    // As discussed, storing pointers to a std::vector while it is actively 
    // growing and reallocating memory will cause a dangling pointer crash. 
    // The Executor now handles the B+ Tree safely on the fly.
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

// --------- LEGACY INDEX FALLBACK ---------
// Because we upgraded to a B+ Tree that uses range_query() instead of search(),
// this old Hash Index function is no longer used by the Executor. 
// We return an empty vector here just to satisfy the compiler if table.h still declares it.
std::vector<Row> Table::get_rows_by_index(const std::string& key) {
    (void)key; // Suppress unused variable warning
    return {}; 
}

// --------- DELETE ---------
int Table::delete_all_rows() {
    int count = rows.size();
    
    // Clear the physical rows
    rows.clear();
    
    // Reset the index by re-instantiating it
    index = Index(); 
    
    return count;
}