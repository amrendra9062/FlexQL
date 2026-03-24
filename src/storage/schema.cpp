#include "storage/schema.h"

Schema::Schema(const std::string& name, const std::vector<Column>& cols)
    : table_name(name), columns(cols) {}

const std::string& Schema::get_table_name() const {
    return table_name;
}

const std::vector<Column>& Schema::get_columns() const {
    return columns;
}

size_t Schema::column_count() const {
    return columns.size();
}