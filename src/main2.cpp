#include "network/client.h"

int main() {
    // Connects to the local server on port 9000
    Client client("127.0.0.1", 9000);
    client.run();
    return 0;
}