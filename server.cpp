#include "connection.hpp"
#include <iostream>
#include <csignal>
#include <cerrno>

namespace {
    volatile std::sig_atomic_t running = true;
    void signal_handler(int) { running = false; }
}

int main() {
    std::signal(SIGINT, signal_handler);

    try {
        net::Server server(net::DEFAULT_PORT);
        std::cout << "Server listening on port " << net::DEFAULT_PORT << "...\n";

        while (running) {
            std::cout << "Waiting for client...\n";
            
            std::optional<net::Socket> client;
            while (running && !client) {
                client = server.try_accept();
            }
            if (!running) break;
            
            std::cout << "Client connected\n";

            while (running) {
                if (auto msg = client->receive()) {
                    std::cout << "Received: " << *msg << "\n";
                    client->send("PONG");
                } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    continue;
                } else {
                    std::cout << "Client disconnected, waiting for new connection...\n";
                    break;
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    std::cout << "Server shutdown\n";
    return 0;
}
