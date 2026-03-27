#include "network/server.h"

int main() {
    // The benchmark explicitly expects the server on port 9000
    Server server(9000);
    server.start();
    return 0;
}