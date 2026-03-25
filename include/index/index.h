#pragma once
#include <unordered_map>
#include <vector>
#include "storage/row.h"

class HashIndex {
private:
    // key → list of rows (handles duplicates)
    std::unordered_map<std::string, std::vector<Row>> index;

public:
    void insert(const std::string& key, const Row& row);

    std::vector<Row> search(const std::string& key);

    bool exists(const std::string& key);
};