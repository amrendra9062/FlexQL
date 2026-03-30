#include "query/executor.h"
#include "index/index.h" // <-- Required for the B+ Tree
#include <ctime>
#include <algorithm>
#include <iostream>
#include <unordered_map>

// ---------- TYPE CASTING HELPER ----------
Value convertValue(const std::string& val, DataType type) {
    if (type == DataType::INT) return std::stoi(val);
    if (type == DataType::DECIMAL) return std::stod(val);
    if (type == DataType::DATETIME) return std::stoll(val);
    
    if (val.size() >= 2 && val.front() == '\'' && val.back() == '\'') {
        return val.substr(1, val.size() - 2);
    }
    return val;
}

// ---------- DOUBLE FORMATTING HELPER ----------
std::string formatDouble(double d) {
    std::string s = std::to_string(d);
    s.erase(s.find_last_not_of('0') + 1, std::string::npos);
    if (s.back() == '.') s.pop_back();
    return s;
}

// ---------- EXECUTE DISPATCHER ----------
std::string Executor::execute(const Query& query) {
    if (query.type == QueryType::CREATE) return executeCreate(query.createQuery);
    if (query.type == QueryType::INSERT) return executeInsert(query.insertQuery);
    if (query.type == QueryType::SELECT) return executeSelect(query.selectQuery);
    if (query.type == QueryType::DELETE) return executeDelete(query.deleteQuery);
    return "Error: Unknown query type\n";
}

// ---------- CREATE ----------
std::string Executor::executeCreate(const CreateQuery& query) {
    if (query.if_not_exists && db.has_table(query.table)) return "Table already exists\n";

    std::vector<Column> cols;
    for (auto& col : query.columns) {
        DataType type = DataType::VARCHAR;
        if (col.second == "INT") type = DataType::INT;
        else if (col.second == "DECIMAL") type = DataType::DECIMAL;
        else if (col.second == "DATETIME") type = DataType::DATETIME;
        cols.push_back({col.first, type});
    }

    db.create_table(Schema(query.table, cols));
    return "Table created\n";
}

// ---------- DELETE ----------
std::string Executor::executeDelete(const DeleteQuery& query) {
    if (!db.has_table(query.table)) return "Error: Table not found\n";
    return std::to_string(db.get_table(query.table).delete_all_rows()) + " rows deleted\n";
}

// ---------- BATCH INSERT ----------
std::string Executor::executeInsert(const InsertQuery& query) {
    if (!db.has_table(query.table)) return "Error: Table not found\n";
    
    Table& table = db.get_table(query.table);
    const auto& cols = table.get_schema().get_columns();

    int inserted = 0;
    for (const auto& row_strs : query.rows) {
        std::vector<Value> values;
        long long expiry = std::time(nullptr) + 31536000; 
        
        for (size_t i = 0; i < row_strs.size() && i < cols.size(); ++i) {
            values.push_back(convertValue(row_strs[i], cols[i].type));
            if (cols[i].name == "EXPIRES_AT") expiry = std::stoll(row_strs[i]);
        }
        table.insert(Row(values, expiry));
        inserted++;
    }
    return std::to_string(inserted) + " rows inserted\n";
}

// ---------- SELECT ENGINE ----------
std::string Executor::executeSelect(const SelectQuery& query) {
    if (!db.has_table(query.table)) return "Error: Table not found\n";
    
    Table& table = db.get_table(query.table);
    auto rows = table.get_all_rows(); 
    const auto& cols = table.get_schema().get_columns();

    // Helper: Map table.column to index
    auto get_col_idx = [&](const std::string& name, const std::vector<Column>& columns) -> int {
        std::string search_name = name;
        size_t dot_pos = search_name.find('.');
        if (dot_pos != std::string::npos) search_name = search_name.substr(dot_pos + 1);
        
        for (size_t i = 0; i < columns.size(); ++i) {
            if (columns[i].name == search_name) return i;
        }
        return -1;
    };

    // --- STANDARD SELECT (NO JOIN) ---
    if (!query.has_join) {
        int where_idx = -1;
        double where_val_num = 0;
        std::string where_val_str = "";
        
        if (query.has_where) {
            where_idx = get_col_idx(query.where.column, cols);
            if (where_idx == -1) return "Error: Column not found in WHERE\n";
            DataType type = cols[where_idx].type;
            if (type == DataType::DECIMAL || type == DataType::INT) where_val_num = std::stod(query.where.value);
            else {
                where_val_str = query.where.value;
                if (where_val_str.front() == '\'') where_val_str = where_val_str.substr(1, where_val_str.size() - 2);
            }
        }

        std::vector<const Row*> matched_rows;

        // [ NEW: B+ TREE RANGE QUERY ACTIVATION ]
        if (query.has_where && (query.where.op == ">" || query.where.op == "<" || query.where.op == ">=" || query.where.op == "<=")) {
            Index range_index; // Creates the B+ Tree
            
            // Build the tree for the queried column
            for (const auto& r : rows) {
                double val = 0;
                std::visit([&](auto&& arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, double> || std::is_same_v<T, int> || std::is_same_v<T, long long>) {
                        val = static_cast<double>(arg);
                    }
                }, r.get_values()[where_idx]);
                range_index.insert(val, &r);
            }

            // Define the bounds based on the SQL operator
            double min_bound = -999999999.0;
            double max_bound = 999999999.0;
            bool inc_min = false, inc_max = false;

            if (query.where.op == ">")  { min_bound = where_val_num; inc_min = false; }
            if (query.where.op == ">=") { min_bound = where_val_num; inc_min = true;  }
            if (query.where.op == "<")  { max_bound = where_val_num; inc_max = false; }
            if (query.where.op == "<=") { max_bound = where_val_num; inc_max = true;  }

            // Execute the O(log N) tree traversal
            matched_rows = range_index.range_query(min_bound, max_bound, inc_min, inc_max);

        } else {
            // [ FALLBACK: LINEAR SCAN FOR '=' OR STRINGS ]
            for (const auto& r : rows) {
                bool match = true;
                if (query.has_where) {
                    std::visit([&](auto&& arg) {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, double> || std::is_same_v<T, int> || std::is_same_v<T, long long>) {
                            double val = static_cast<double>(arg);
                            if (query.where.op == "=" && val != where_val_num) match = false;
                        } 
                        else if constexpr (std::is_same_v<T, std::string>) {
                            if (query.where.op == "=" && arg != where_val_str) match = false;
                        }
                    }, r.get_values()[where_idx]);
                }
                if (match) matched_rows.push_back(&r);
            }
        }

        // --- ORDER BY LOGIC ---
        int order_idx = -1;
        if (query.has_order_by) {
            order_idx = get_col_idx(query.order_by.column, cols);
            if (order_idx == -1) return "Error: Column not found in ORDER BY\n";
            std::sort(matched_rows.begin(), matched_rows.end(), [&](const Row* a, const Row* b) {
                bool res = false;
                std::visit([&](auto&& val_a, auto&& val_b) {
                    using T_A = std::decay_t<decltype(val_a)>;
                    using T_B = std::decay_t<decltype(val_b)>;
                    if constexpr (std::is_same_v<T_A, T_B>) {
                        res = query.order_by.descending ? (val_a > val_b) : (val_a < val_b);
                    }
                }, a->get_values()[order_idx], b->get_values()[order_idx]);
                return res;
            });
        }

        // --- SELECT RESULTS FORMATTING ---
        std::vector<int> select_indices;
        if (query.select_all) {
            for (size_t i = 0; i < cols.size(); ++i) select_indices.push_back(i);
        } else {
            for (const auto& col_name : query.columns) {
                int idx = get_col_idx(col_name, cols);
                if (idx == -1) return "Error: Column not found in SELECT\n";
                select_indices.push_back(idx);
            }
        }

        std::string result;
        for (const Row* r : matched_rows) {
            for (size_t i = 0; i < select_indices.size(); ++i) {
                std::visit([&](auto&& arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, std::string>) result += arg;
                    else if constexpr (std::is_same_v<T, double>) result += formatDouble(arg);
                    else result += std::to_string(arg);
                }, r->get_values()[select_indices[i]]);
                if (i < select_indices.size() - 1) result += " ";
            }
            result += "\n";
        }
        return result;
    }

    // --- INNER JOIN LOGIC ---
    if (!db.has_table(query.join_table)) return "Error: Join table not found\n";
    Table& right_table = db.get_table(query.join_table);
    auto right_rows = right_table.get_all_rows();
    const auto& right_cols = right_table.get_schema().get_columns();

    int left_join_idx = get_col_idx(query.join_condition.left_column, cols);
    int right_join_idx = get_col_idx(query.join_condition.right_column, right_cols);
    if (left_join_idx == -1 || right_join_idx == -1) return "Error: Join condition column missing\n";

    std::unordered_map<std::string, std::vector<const Row*>> hash_join;
    for (const auto& r : right_rows) {
        std::string key;
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::string>) key = arg;
            else if constexpr (std::is_same_v<T, double>) key = formatDouble(arg);
            else key = std::to_string(arg);
        }, r.get_values()[right_join_idx]);
        hash_join[key].push_back(&r);
    }

    std::vector<std::pair<int, bool>> select_layout; 
    for (const auto& col_name : query.columns) {
        std::string table_prefix = col_name.substr(0, col_name.find('.'));
        std::string raw_col = col_name.substr(col_name.find('.') + 1);
        
        int idx = -1;
        bool is_left = true;
        if (table_prefix == query.table) idx = get_col_idx(raw_col, cols);
        else { idx = get_col_idx(raw_col, right_cols); is_left = false; }
        
        if (idx == -1) return "Error: Column not found in SELECT\n";
        select_layout.push_back({idx, is_left});
    }

    int join_where_idx = -1;
    bool join_where_is_left = true;
    double j_where_num = 0;
    if (query.has_where) {
        std::string table_prefix = query.where.column.substr(0, query.where.column.find('.'));
        std::string raw_col = query.where.column.substr(query.where.column.find('.') + 1);
        
        if (table_prefix == query.table) { join_where_idx = get_col_idx(raw_col, cols); } 
        else { join_where_idx = get_col_idx(raw_col, right_cols); join_where_is_left = false; }
        
        if (join_where_idx == -1) return "Error: Column not found in WHERE\n";
        j_where_num = std::stod(query.where.value);
    }
    
    int join_order_idx = -1;
    bool join_order_is_left = true;
    if (query.has_order_by) {
        std::string table_prefix = query.order_by.column.substr(0, query.order_by.column.find('.'));
        std::string raw_col = query.order_by.column.substr(query.order_by.column.find('.') + 1);
        
        if (table_prefix == query.table) { join_order_idx = get_col_idx(raw_col, cols); } 
        else { join_order_idx = get_col_idx(raw_col, right_cols); join_order_is_left = false; }
        
        if (join_order_idx == -1) return "Error: Column not found in ORDER BY\n";
    }

    struct JoinedRow { const Row* left; const Row* right; };
    std::vector<JoinedRow> joined_results;

    for (const auto& l_row : rows) {
        std::string key;
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::string>) key = arg;
            else if constexpr (std::is_same_v<T, double>) key = formatDouble(arg);
            else key = std::to_string(arg);
        }, l_row.get_values()[left_join_idx]);

        if (hash_join.count(key)) {
            for (const Row* r_row : hash_join[key]) {
                bool match = true;
                if (query.has_where) {
                    const Row* target_row = join_where_is_left ? &l_row : r_row;
                    std::visit([&](auto&& arg) {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, double> || std::is_same_v<T, int> || std::is_same_v<T, long long>) {
                            double val = static_cast<double>(arg);
                            if (query.where.op == "=" && val != j_where_num) match = false;
                            else if (query.where.op == ">" && val <= j_where_num) match = false;
                            else if (query.where.op == ">=" && val < j_where_num) match = false;
                        }
                    }, target_row->get_values()[join_where_idx]);
                }
                if (match) joined_results.push_back({&l_row, r_row});
            }
        }
    }
    
    if (query.has_order_by) {
        std::sort(joined_results.begin(), joined_results.end(), [&](const JoinedRow& a, const JoinedRow& b) {
            bool res = false;
            const Row* target_a = join_order_is_left ? a.left : a.right;
            const Row* target_b = join_order_is_left ? b.left : b.right;
            
            std::visit([&](auto&& val_a, auto&& val_b) {
                using T_A = std::decay_t<decltype(val_a)>;
                using T_B = std::decay_t<decltype(val_b)>;
                if constexpr (std::is_same_v<T_A, T_B>) {
                    res = query.order_by.descending ? (val_a > val_b) : (val_a < val_b);
                }
            }, target_a->get_values()[join_order_idx], target_b->get_values()[join_order_idx]);
            return res;
        });
    }

    std::string result;
    for (const auto& j_row : joined_results) {
        for (size_t i = 0; i < select_layout.size(); ++i) {
            const Row* target = select_layout[i].second ? j_row.left : j_row.right;
            std::visit([&](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::string>) result += arg;
                else if constexpr (std::is_same_v<T, double>) result += formatDouble(arg);
                else result += std::to_string(arg);
            }, target->get_values()[select_layout[i].first]);
            
            if (i < select_layout.size() - 1) result += " ";
        }
        result += "\n";
    }

    return result;
}