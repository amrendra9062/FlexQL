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

    void execute(const Query& query);

private:
    void executeCreate(const CreateQuery& query);
    void executeInsert(const InsertQuery& query);
    void executeSelect(const SelectQuery& query);
};