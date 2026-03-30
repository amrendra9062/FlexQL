#pragma once

#include <vector>
#include <algorithm>
#include "storage/row.h"

// The maximum number of children a node can have. 
const int BPT_ORDER = 32; 

struct BPTNode {
    bool is_leaf;
    std::vector<double> keys;
    std::vector<BPTNode*> children; 
    std::vector<std::vector<const Row*>> values; // Array of rows for duplicate keys
    BPTNode* next; // The magic link for range queries!

    BPTNode(bool leaf) : is_leaf(leaf), next(nullptr) {}
};

class BPlusTree {
private:
    BPTNode* root;
    void insertInternal(double key, BPTNode* cursor, BPTNode* child);
    BPTNode* findParent(BPTNode* cursor, BPTNode* child);

public:
    BPlusTree();
    void insert(double key, const Row* row);
    
    // The superpower of the B+ Tree
    std::vector<const Row*> findRange(double min_val, double max_val, bool include_min, bool include_max);
};

// Wrapper for the Table to use
class Index {
    BPlusTree bpt;
public:
    void insert(double key, const Row* row);
    std::vector<const Row*> range_query(double min, double max, bool inc_min, bool inc_max);
};