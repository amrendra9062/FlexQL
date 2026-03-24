#include "storage/row.h"

Row::Row(const std::vector<Value>& vals, long long expiry)
    : values(vals), expiration_time(expiry) {}

const std::vector<Value>& Row::get_values() const {
    return values;
}

long long Row::get_expiration_time() const {
    return expiration_time;
}