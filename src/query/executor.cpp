#include "query/executor.h"
#include <iostream>
#include <ctime>
#include <type_traits>
#include <unordered_map>
#include <chrono>

// helper
Value convertValue(const std::string& val) {
    if (!val.empty() && std::isdigit(val[0])) {
        return std::stoi(val);
    }
    return val;
}


void Executor::execute(const Query& query) {
    switch (query.type) {
        case QueryType::CREATE:
            executeCreate(query.createQuery);
            break;
        case QueryType::INSERT:
            executeInsert(query.insertQuery);
            break;
        case QueryType::SELECT:
            executeSelect(query.selectQuery);
            break;
        default:
            throw std::runtime_error("Unknown query type");
    }
}

void Executor::executeCreate(const CreateQuery& query) {
    std::vector<Column> cols;

    for (auto& col : query.columns) {
        DataType type;

        if (col.second == "INT") type = DataType::INT;
        else if (col.second == "VARCHAR") type = DataType::VARCHAR;
        else type = DataType::VARCHAR;

        cols.push_back({col.first, type});
    }

    Schema schema(query.table, cols);
    db.create_table(schema);

    std::cout << "Table created\n";
}

void Executor::executeInsert(const InsertQuery& query) {
    std::vector<Value> values;

    for (auto& v : query.values) {
        values.push_back(convertValue(v));
    }

    long long expiry = std::time(nullptr) + 1000;

    Row row(values, expiry);
    db.get_table(query.table).insert(row);

    std::cout << "Row inserted\n";
}

void Executor::executeSelect(const SelectQuery& query) {
    auto start = std::chrono::high_resolution_clock::now();

    std::string cache_key = query.table;

    if (query.has_where) {
        cache_key += "_WHERE_" + query.where.column + "_" + query.where.value;
    }

    // ------------------ CACHE HIT ------------------
    if (cache.exists(cache_key)) {
        auto rows = cache.get(cache_key);

        std::cout << "Result (from cache):\n";

        for (const auto& r : rows) {
            for (const auto& v : r.get_values()) {
                std::visit([](auto&& arg) {
                    std::cout << arg << " ";
                }, v);
            }
            std::cout << "\n";
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Time: "
                  << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
                  << " us\n";

        return;
    }

    std::cout << "Result (computed):\n";
    std::vector<Row> result_rows;

    // ------------------ NO JOIN ------------------
    if (!query.has_join) {
        auto& table = db.get_table(query.table);
        const auto& schema = table.get_schema();

        // -------- INDEX --------
        if (query.has_where && query.where.column == schema.get_columns()[0].name) {
            auto indexed_rows = table.get_rows_by_index(query.where.value);

            for (const auto& r : indexed_rows) {
                result_rows.push_back(r);

                for (const auto& v : r.get_values()) {
                    std::visit([](auto&& arg) {
                        std::cout << arg << " ";
                    }, v);
                }
                std::cout << "\n";
            }

            cache.put(cache_key, result_rows);

            auto end = std::chrono::high_resolution_clock::now();
            std::cout << "Time: "
                      << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
                      << " us\n";

            return;
        }

        auto rows = table.get_all_rows();

        int condition_idx = -1;

        if (query.has_where) {
            for (size_t i = 0; i < schema.get_columns().size(); i++) {
                if (schema.get_columns()[i].name == query.where.column) {
                    condition_idx = i;
                    break;
                }
            }
        }

        for (const auto& r : rows) {
            bool match = true;

            if (query.has_where) {
                std::string val_str;

                std::visit([&](auto&& arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, std::string>) {
                        val_str = arg;
                    } else {
                        val_str = std::to_string(arg);
                    }
                }, r.get_values()[condition_idx]);

                if (val_str != query.where.value) {
                    match = false;
                }
            }

            if (match) {
                result_rows.push_back(r);

                for (const auto& v : r.get_values()) {
                    std::visit([](auto&& arg) {
                        std::cout << arg << " ";
                    }, v);
                }
                std::cout << "\n";
            }
        }

        cache.put(cache_key, result_rows);
    }

    // ------------------ HASH JOIN ------------------
    else {
        auto& left_table = db.get_table(query.table);
        auto& right_table = db.get_table(query.join_table);

        auto left_rows = left_table.get_all_rows();
        auto right_rows = right_table.get_all_rows();

        const auto& left_schema = left_table.get_schema();
        const auto& right_schema = right_table.get_schema();

        int left_idx = -1;
        int right_idx = -1;

        for (size_t i = 0; i < left_schema.get_columns().size(); i++) {
            if (left_schema.get_columns()[i].name == query.join_condition.left_column) {
                left_idx = i;
                break;
            }
        }

        for (size_t i = 0; i < right_schema.get_columns().size(); i++) {
            if (right_schema.get_columns()[i].name == query.join_condition.right_column) {
                right_idx = i;
                break;
            }
        }

        std::unordered_map<std::string, std::vector<Row>> hash_table;

        for (const auto& r : right_rows) {
            std::string key;

            std::visit([&](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::string>) key = arg;
                else key = std::to_string(arg);
            }, r.get_values()[right_idx]);

            hash_table[key].push_back(r);
        }

        for (const auto& lrow : left_rows) {
            std::string key;

            std::visit([&](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::string>) key = arg;
                else key = std::to_string(arg);
            }, lrow.get_values()[left_idx]);

            if (hash_table.find(key) != hash_table.end()) {
                for (const auto& rrow : hash_table[key]) {

                    result_rows.push_back(lrow);

                    for (const auto& v : lrow.get_values()) {
                        std::visit([](auto&& arg) {
                            std::cout << arg << " ";
                        }, v);
                    }

                    for (const auto& v : rrow.get_values()) {
                        std::visit([](auto&& arg) {
                            std::cout << arg << " ";
                        }, v);
                    }

                    std::cout << "\n";
                }
            }
        }

        cache.put(cache_key, result_rows);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Time: "
              << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
              << " us\n";
}