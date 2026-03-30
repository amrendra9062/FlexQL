#include "network/server.h"
#include "parser/tokenizer.h"
#include "parser/parser.h"

#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <cstring>
#include <thread>
#include <fstream>
#include <string>
#include <mutex>

Server::Server(int port) : port(port), executor(db) {}

void Server::start() {
    // =========================================================
    // 1. FAULT TOLERANCE: RESTORE FROM WRITE-AHEAD LOG (WAL)
    // =========================================================
    std::ifstream wal_in("flexql.wal");
    if (wal_in.is_open()) {
        std::cout << "[Recovery] Restoring database from persistent WAL...\n";
        std::string single_query;
        int restored_queries = 0;
        
        // Read the log file query by query (split by semicolon)
        while (std::getline(wal_in, single_query, ';')) {
            // Skip empty lines or pure whitespace
            if (single_query.find_first_not_of(" \n\r\t") == std::string::npos) continue;
            
            single_query += ";"; // Put the semicolon back for the parser
            
            try {
                Tokenizer tokenizer(single_query);
                auto tokens = tokenizer.tokenize();
                if (!tokens.empty()) {
                    Parser parser(tokens);
                    Query q = parser.parse();
                    // Execute silently to rebuild RAM state
                    executor.execute(q);
                    restored_queries++;
                }
            } catch (...) {
                // Ignore any malformed lines caused by severe crashes
            }
        }
        std::cout << "[Recovery] Restored " << restored_queries << " persistent operations.\n";
    } else {
        std::cout << "[Recovery] No persistent WAL found. Starting fresh database.\n";
    }
    // =========================================================

    // --- START SERVER ---
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Server bind failed on port " << port << "\n";
        return;
    }
    
    listen(server_fd, 10);
    std::cout << "Server running on port " << port << "\n";

    // --- ACCEPT CONNECTIONS ---
    while (true) {
        int addrlen = sizeof(address);
        int client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);

        if (client_socket < 0) continue;

        std::thread([this, client_socket]() {
            char buffer[16384]; 
            
            while (true) { 
                std::string query_str;
                bool query_complete = false;
                
                // Network Accumulator
                while (!query_complete) {
                    memset(buffer, 0, sizeof(buffer));
                    int bytes = read(client_socket, buffer, sizeof(buffer) - 1);
                    
                    if (bytes <= 0) {
                        close(client_socket);
                        return; 
                    }
                    
                    query_str += buffer;
                    
                    size_t last_char = query_str.find_last_not_of(" \n\r\t");
                    if (last_char != std::string::npos && query_str[last_char] == ';') {
                        query_complete = true;
                    }
                }

                std::string result;
                try {
                    Tokenizer tokenizer(query_str);
                    auto tokens = tokenizer.tokenize();

                    if (tokens.empty()) {
                        result = "END\n";
                        send(client_socket, result.c_str(), result.size(), 0);
                        continue;
                    }

                    Parser parser(tokens);
                    Query q = parser.parse();

                    // Check if query is a mutation BEFORE execution
                    std::string cmd = tokens[0].value;
                    for (char &c : cmd) c = toupper(c);
                    bool is_mutation = (cmd == "CREATE" || cmd == "INSERT" || cmd == "DELETE");

                    // THREAD-SAFE EXECUTION
                    std::lock_guard<std::mutex> lock(db_mutex);
                    result = executor.execute(q);

                    // =========================================================
                    // 2. FAULT TOLERANCE: APPEND TO WRITE-AHEAD LOG (WAL)
                    // =========================================================
                    // If the query changed data AND didn't throw an error, save it.
                    if (is_mutation && result.find("Error") == std::string::npos) {
                        std::ofstream wal_out("flexql.wal", std::ios::app);
                        wal_out << query_str;
                        if (query_str.back() != '\n') wal_out << "\n";
                    }
                    // =========================================================
                }
                catch (const std::exception& e) {
                    result = std::string("Error: ") + e.what() + "\n";
                }

                result += "END\n"; 
                send(client_socket, result.c_str(), result.size(), 0);
            }
        }).detach(); 
    }
}