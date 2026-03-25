#include "cache/cache.h"

LRUCache::LRUCache(int cap) : capacity(cap) {}

bool LRUCache::exists(const std::string& key) {
    return cache.find(key) != cache.end();
}

std::vector<Row> LRUCache::get(const std::string& key) {
    auto it = cache.find(key);

    if (it == cache.end()) return {};

    // move to front
    order.erase(it->second.first);
    order.push_front(key);

    it->second.first = order.begin();

    return it->second.second;
}

void LRUCache::put(const std::string& key, const std::vector<Row>& value) {
    if (cache.find(key) != cache.end()) {
        order.erase(cache[key].first);
    } else if (cache.size() >= capacity) {
        std::string lru = order.back();
        order.pop_back();
        cache.erase(lru);
    }

    order.push_front(key);
    cache[key] = {order.begin(), value};
}