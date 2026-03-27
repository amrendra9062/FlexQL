#include "network/client.h"
#include "flexql.h" // Crucial: We are now using the C-API wrapper
#include <iostream>
#include <string>

Client::Client(const std::string& host, int port) : host(host), port(port) {}

// Standard SQLite-style callback for printing rows
int repl_callback(void *data, int argc, char **argv, char **azColName) {
    (void)data;
    (void)azColName; // Suppress unused warning (we can add column headers later)
    
    for (int i = 0; i < argc; i++) {
        std::cout << (argv[i] ? argv[i] : "NULL") << " ";
    }
    std::cout << "\n";
    return 0;
}

void Client::run() {
    FlexQL* db = nullptr;
    
    // Use the API to connect
    if (flexql_open(host.c_str(), port, &db) != FLEXQL_OK) {
        std::cerr << "Cannot connect to FlexQL server\n";
        return;
    }
    
    std::cout << "Connected to FlexQL server\n";

    while (true) {
        std::string query;
        std::cout << "flexql> ";
        std::getline(std::cin, query);

        if (query == "exit" || query == ".exit") {
            break;
        }
        if (query.empty()) continue;

        char* errMsg = nullptr;
        
        // Use the API to execute queries
        int rc = flexql_exec(db, query.c_str(), repl_callback, nullptr, &errMsg);

        if (rc != FLEXQL_OK) {
            std::cout << "SQL error: " << (errMsg ? errMsg : "Unknown") << "\n";
            if (errMsg) flexql_free(errMsg);
        }
    }

    // Use the API to clean up
    flexql_close(db);
    std::cout << "Connection closed\n";
}