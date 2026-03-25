#pragma once
#include <unordered_map>
#include <list>
#include <string>
#include <vector>
#include "storage/row.h"

class LRUCache {
private:
    int capacity;

    // key → (iterator, value)
    std::unordered_map<std::string, std::pair<std::list<std::string>::iterator, std::vector<Row>>> cache;

    std::list<std::string> order; // most recent at front

public:
    LRUCache(int cap);

    bool exists(const std::string& key);

    std::vector<Row> get(const std::string& key);

    void put(const std::string& key, const std::vector<Row>& value);
};