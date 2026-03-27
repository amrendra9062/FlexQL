#include "flexql.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstdlib>

struct FlexQL {
    std::string host;
    int port;
    int sock_fd; // We will use a persistent connection to fix the latency issue!
};

int flexql_open(const char *host, int port, FlexQL **db) {
    *db = new FlexQL{host, port, -1};

    // Open a persistent socket
    (*db)->sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &serv_addr.sin_addr);

    if (connect((*db)->sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        delete *db;
        *db = nullptr;
        return FLEXQL_ERROR;
    }
    return FLEXQL_OK;
}

int flexql_close(FlexQL *db) {
    if (db) {
        if (db->sock_fd >= 0) {
            close(db->sock_fd);
        }
        delete db;
    }
    return FLEXQL_OK;
}

void flexql_free(void *ptr) {
    if (ptr) {
        free(ptr);
    }
}

int flexql_exec(FlexQL *db, const char *sql, flexql_callback cb, void *data, char **errMsg) {
    if (!db || db->sock_fd < 0) return FLEXQL_ERROR;

    // Send the query
    std::string query(sql);
    send(db->sock_fd, query.c_str(), query.size(), 0);

    // Read the response
    std::string response;
    char buffer[1024];
    while (true) {
        int bytes = read(db->sock_fd, buffer, sizeof(buffer) - 1);
        if (bytes <= 0) break;
        buffer[bytes] = '\0';
        response += buffer;
        if (response.find("END\n") != std::string::npos) break;
    }

    // Check for errors returned by your server
    if (response.find("Error:") != std::string::npos || response.find("Unknown") != std::string::npos) {
        if (errMsg) {
            std::string err = response.substr(0, response.find("END\n"));
            *errMsg = strdup(err.c_str());
        }
        return FLEXQL_ERROR;
    }

    // If there is a callback, parse the result and invoke it.
    // (Note: Currently your server returns space-separated values. 
    // We will need to upgrade the server output format later, but this parses the current setup.)
    if (cb) {
        std::istringstream iss(response);
        std::string line;
        while (std::getline(iss, line)) {
            if (line == "END" || line.find("Result") != std::string::npos) continue;
            if (line.empty()) continue;

            std::vector<char*> argv;
            std::istringstream lineStream(line);
            std::string token;
            while (lineStream >> token) {
                argv.push_back(strdup(token.c_str()));
            }

            if (!argv.empty()) {
                // We don't have column names yet in your server output, so passing nullptr for azColName
                cb(data, argv.size(), argv.data(), nullptr);
            }

            // Cleanup row memory
            for (char* arg : argv) { free(arg); }
        }
    }

    return FLEXQL_OK;
}