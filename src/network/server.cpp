#include "network/server.h"
#include "parser/tokenizer.h"
#include "parser/parser.h"

#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <cstring>
#include <thread>

Server::Server(int port) : port(port), executor(db) {}

void Server::start() {
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

    while (true) {
        int addrlen = sizeof(address);
        int client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);

        if (client_socket < 0) continue;

        std::thread([this, client_socket]() {
            char buffer[16384]; // Increased chunk size to 16KB for efficiency
            
            while (true) { 
                std::string query_str;
                bool query_complete = false;
                
                // --- THE ACCUMULATOR LOOP ---
                // Keep reading chunks from the network until we see the ';'
                while (!query_complete) {
                    memset(buffer, 0, sizeof(buffer));
                    int bytes = read(client_socket, buffer, sizeof(buffer) - 1);
                    
                    if (bytes <= 0) {
                        close(client_socket);
                        return; // Client disconnected, kill thread
                    }
                    
                    query_str += buffer;
                    
                    // Ignore trailing whitespace/newlines to accurately check for the ';'
                    size_t last_char = query_str.find_last_not_of(" \n\r\t");
                    if (last_char != std::string::npos && query_str[last_char] == ';') {
                        query_complete = true;
                    }
                }

                std::string result;
                try {
                    Tokenizer tokenizer(query_str);
                    auto tokens = tokenizer.tokenize();

                    Parser parser(tokens);
                    Query q = parser.parse();

                    std::lock_guard<std::mutex> lock(db_mutex);
                    result = executor.execute(q);
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