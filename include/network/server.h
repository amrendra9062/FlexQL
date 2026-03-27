#pragma once
#include "storage/database.h"
#include "query/executor.h"
#include <mutex>

class Server {
private:
    int port;
    Database db;
    Executor executor;
    std::mutex db_mutex; // Required for safe concurrent access

public:
    Server(int port);
    void start();
};