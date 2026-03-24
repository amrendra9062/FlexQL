#pragma once
#include <string>
#include <vector>

enum class DataType {
    INT,
    DECIMAL,
    VARCHAR,
    DATETIME
};

struct Column {
    std::string name;
    DataType type;
};

class Schema {
private:
    std::string table_name;
    std::vector<Column> columns;

public:
    Schema(const std::string& name, const std::vector<Column>& cols);

    const std::string& get_table_name() const;
    const std::vector<Column>& get_columns() const;

    size_t column_count() const;
};