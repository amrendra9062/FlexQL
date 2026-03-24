#include "parser/tokenizer.h"
#include "parser/parser.h"
#include "storage/database.h"

#include <iostream>
#include <ctime>
#include <string>
#include <vector>

// Helper: convert string → Value (simple version)
Value convertValue(const std::string& val) {
    // if number
    if (!val.empty() && std::isdigit(val[0])) {
        return std::stoi(val);
    }
    // otherwise string
    return val;
}

int main() {
    try {
        Database db;

        // -------------------------------
        // 1. CREATE TABLE
        // -------------------------------
        std::string create_sql = "CREATE TABLE student (id INT, name VARCHAR)";

        Tokenizer t1(create_sql);
        auto tokens1 = t1.tokenize();

        Parser p1(tokens1);
        Query q1 = p1.parse();

        std::vector<Column> cols;
        for (auto& col : q1.createQuery.columns) {
            DataType type;

            if (col.second == "INT") type = DataType::INT;
            else if (col.second == "VARCHAR") type = DataType::VARCHAR;
            else type = DataType::VARCHAR; // fallback

            cols.push_back({col.first, type});
        }

        Schema schema(q1.createQuery.table, cols);
        db.create_table(schema);

        std::cout << "Table created\n";

        // -------------------------------
        // 2. INSERT
        // -------------------------------
        std::string insert_sql = "INSERT INTO student VALUES (1, 'Alice')";

        Tokenizer t2(insert_sql);
        auto tokens2 = t2.tokenize();

        Parser p2(tokens2);
        Query q2 = p2.parse();

        std::vector<Value> values;
        for (auto& v : q2.insertQuery.values) {
            values.push_back(convertValue(v));
        }

        long long expiry = std::time(nullptr) + 1000;

        Row row(values, expiry);
        db.get_table(q2.insertQuery.table).insert(row);

        std::cout << "Row inserted\n";

        // -------------------------------
        // 3. SELECT
        // -------------------------------
        std::string select_sql = "SELECT * FROM student";

        Tokenizer t3(select_sql);
        auto tokens3 = t3.tokenize();

        Parser p3(tokens3);
        Query q3 = p3.parse();

        auto rows = db.get_table(q3.selectQuery.table).get_all_rows();

        std::cout << "Result:\n";

        for (const auto& r : rows) {
            for (const auto& val : r.get_values()) {
                std::visit([](auto&& arg) {
                    std::cout << arg << " ";
                }, val);
            }
            std::cout << "\n";
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }

    return 0;
}