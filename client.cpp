#include "connection.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <csignal>

namespace {
    volatile std::sig_atomic_t running = true;
    void signal_handler(int) { running = false; }
}

int main() {
    std::signal(SIGINT, signal_handler);

    try {
        auto socket = net::connect(net::LOCALHOST, net::DEFAULT_PORT);
        std::cout << "Connected to server\n";

        while (running) {
            socket.send("PING");
            std::cout << "Sent: PING\n";

            if (auto response = socket.receive()) {
                std::cout << "Received: " << *response << "\n\n";
            } else {
                std::cout << "Server disconnected\n";
                break;
            }

            std::this_thread::sleep_for(net::PING_INTERVAL);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    std::cout << "Client shutdown\n";
    return 0;
}
