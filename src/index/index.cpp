#include "index/index.h"
#include <iostream>

BPlusTree::BPlusTree() {
    root = new BPTNode(true);
}

void BPlusTree::insert(double key, const Row* row) {
    if (root->keys.empty()) {
        root->keys.push_back(key);
        root->values.push_back({row});
        return;
    }

    BPTNode* cursor = root;
    BPTNode* parent = nullptr;

    // Traverse to the leaf
    while (!cursor->is_leaf) {
        parent = cursor;
        auto it = std::upper_bound(cursor->keys.begin(), cursor->keys.end(), key);
        int idx = std::distance(cursor->keys.begin(), it);
        cursor = cursor->children[idx];
    }

    // Check for duplicate key
    auto it = std::lower_bound(cursor->keys.begin(), cursor->keys.end(), key);
    int idx = std::distance(cursor->keys.begin(), it);
    
    if (idx < (int)cursor->keys.size() && cursor->keys[idx] == key) {
        cursor->values[idx].push_back(row);
        return;
    }

    // Insert into leaf
    cursor->keys.insert(cursor->keys.begin() + idx, key);
    cursor->values.insert(cursor->values.begin() + idx, {row});

    // Split if overflow
    if (cursor->keys.size() >= BPT_ORDER) {
        BPTNode* newLeaf = new BPTNode(true);
        int mid = BPT_ORDER / 2;

        newLeaf->keys.assign(cursor->keys.begin() + mid, cursor->keys.end());
        newLeaf->values.assign(cursor->values.begin() + mid, cursor->values.end());
        
        cursor->keys.erase(cursor->keys.begin() + mid, cursor->keys.end());
        cursor->values.erase(cursor->values.begin() + mid, cursor->values.end());

        // Link the leaves!
        newLeaf->next = cursor->next;
        cursor->next = newLeaf;

        if (cursor == root) {
            BPTNode* newRoot = new BPTNode(false);
            newRoot->keys.push_back(newLeaf->keys[0]);
            newRoot->children.push_back(cursor);
            newRoot->children.push_back(newLeaf);
            root = newRoot;
        } else {
            insertInternal(newLeaf->keys[0], parent, newLeaf);
        }
    }
}

void BPlusTree::insertInternal(double key, BPTNode* cursor, BPTNode* child) {
    auto it = std::upper_bound(cursor->keys.begin(), cursor->keys.end(), key);
    int idx = std::distance(cursor->keys.begin(), it);

    cursor->keys.insert(cursor->keys.begin() + idx, key);
    cursor->children.insert(cursor->children.begin() + idx + 1, child);

    if (cursor->keys.size() >= BPT_ORDER) {
        BPTNode* newInternal = new BPTNode(false);
        int mid = BPT_ORDER / 2;
        double upKey = cursor->keys[mid];

        newInternal->keys.assign(cursor->keys.begin() + mid + 1, cursor->keys.end());
        newInternal->children.assign(cursor->children.begin() + mid + 1, cursor->children.end());

        cursor->keys.erase(cursor->keys.begin() + mid, cursor->keys.end());
        cursor->children.erase(cursor->children.begin() + mid + 1, cursor->children.end());

        if (cursor == root) {
            BPTNode* newRoot = new BPTNode(false);
            newRoot->keys.push_back(upKey);
            newRoot->children.push_back(cursor);
            newRoot->children.push_back(newInternal);
            root = newRoot;
        } else {
            insertInternal(upKey, findParent(root, cursor), newInternal);
        }
    }
}

BPTNode* BPlusTree::findParent(BPTNode* cursor, BPTNode* child) {
    if (cursor->is_leaf || cursor->children[0]->is_leaf) return nullptr;
    for (size_t i = 0; i < cursor->children.size(); i++) {
        if (cursor->children[i] == child) return cursor;
        BPTNode* parent = findParent(cursor->children[i], child);
        if (parent != nullptr) return parent;
    }
    return nullptr;
}

std::vector<const Row*> BPlusTree::findRange(double min_val, double max_val, bool include_min, bool include_max) {
    std::vector<const Row*> results;
    if (root->keys.empty()) return results;

    BPTNode* cursor = root;
    // 1. O(log N) traversal down to the correct leaf
    while (!cursor->is_leaf) {
        auto it = std::upper_bound(cursor->keys.begin(), cursor->keys.end(), min_val);
        int idx = std::distance(cursor->keys.begin(), it);
        cursor = cursor->children[idx];
    }

    // 2. Linear walk across the linked leaves
    while (cursor != nullptr) {
        for (size_t i = 0; i < cursor->keys.size(); i++) {
            double k = cursor->keys[i];
            
            if (k > max_val || (k == max_val && !include_max)) {
                return results; // We have passed the range, stop searching!
            }
            
            if (k > min_val || (k == min_val && include_min)) {
                for (const Row* r : cursor->values[i]) {
                    results.push_back(r);
                }
            }
        }
        cursor = cursor->next; // Move to the adjacent leaf
    }
    return results;
}

// Wrapper methods
void Index::insert(double key, const Row* row) { 
    bpt.insert(key, row); 
}

std::vector<const Row*> Index::range_query(double min, double max, bool inc_min, bool inc_max) {
    return bpt.findRange(min, max, inc_min, inc_max); // Fixed call to findRange!
}