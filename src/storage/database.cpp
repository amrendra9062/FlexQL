#include "storage/database.h"
#include <stdexcept>

void Database::create_table(const Schema& schema) {
    const std::string& name = schema.get_table_name();

    if (tables.find(name) != tables.end()) {
        throw std::runtime_error("Table already exists");
    }

    tables.emplace(name, Table(schema));
}

Table& Database::get_table(const std::string& name) {
    if (tables.find(name) == tables.end()) {
        throw std::runtime_error("Table not found");
    }

    return tables.at(name);
}

bool Database::has_table(const std::string& name) const {
    return tables.find(name) != tables.end();
}