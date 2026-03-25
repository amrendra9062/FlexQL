#include "index/index.h"

void HashIndex::insert(const std::string& key, const Row& row) {
    index[key].push_back(row);
}

std::vector<Row> HashIndex::search(const std::string& key) {
    if (index.find(key) != index.end()) {
        return index[key];
    }
    return {};
}

bool HashIndex::exists(const std::string& key) {
    return index.find(key) != index.end();
}