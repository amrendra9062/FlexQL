#pragma once
#include "parser/ast.h"
#include "storage/database.h"
#include "cache/cache.h"

class Executor {
private:
    Database& db;
    LRUCache cache;

public:
    Executor(Database& database) : db(database), cache(10) {}

    std::string execute(const Query& query);

private:
    std::string executeCreate(const CreateQuery& query);
    std::string executeInsert(const InsertQuery& query);
    std::string executeSelect(const SelectQuery& query);
    std::string executeDelete(const DeleteQuery& query); // NEW
};