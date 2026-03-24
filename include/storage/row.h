#pragma once
#include <variant>
#include <vector>
#include <string>

using Value = std::variant<int, double, std::string, long long>;

class Row {
private:
    std::vector<Value> values;
    long long expiration_time; // epoch time

public:
    Row(const std::vector<Value>& vals, long long expiry);

    const std::vector<Value>& get_values() const;
    long long get_expiration_time() const;
};