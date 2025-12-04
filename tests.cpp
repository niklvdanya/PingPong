#include "connection.hpp"

#include <cassert>
#include <iostream>
#include <thread>

void test_socket_creation() {
    net::Socket sock;
    assert(sock.fd() >= 0);
}

void test_socket_move() {
    net::Socket sock1;
    int fd = sock1.fd();
    net::Socket sock2 = std::move(sock1);
    assert(sock2.fd() == fd);
    assert(sock1.fd() == -1);
}

void test_address_creation() {
    net::Address addr("127.0.0.1", 8080);
    assert(addr.ptr() != nullptr);
    assert(addr.size() == sizeof(sockaddr_in));
}

void test_server_bind() {
    net::Server server(9999);
    net::Socket client;
    net::Address addr("127.0.0.1", 9999);
    assert(connect(client.fd(), addr.ptr(), addr.size()) == 0);
}

void test_ping_pong() {
    constexpr uint16_t port = 9998;
    std::thread server_thread([port]() {
        net::Server server(port);
        auto client = server.accept();
        auto msg = client.receive();
        assert(msg.has_value());
        assert(*msg == "PING");
        client.send("PONG");
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto client = net::connect("127.0.0.1", port);
    client.send("PING");
    auto response = client.receive();
    assert(response.has_value());
    assert(*response == "PONG");

    server_thread.join();
}

int main() {
    test_socket_creation();
    test_socket_move();
    test_address_creation();
    test_server_bind();
    test_ping_pong();

    std::cout << "All tests passed\n";
    return 0;
}
