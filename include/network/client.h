#pragma once
#include <string>

class Client {
private:
    std::string host;
    int port;

public:
    Client(const std::string& host, int port);
    void run();
};