#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <string>
#include <string_view>
#include <chrono>
#include <stdexcept>
#include <utility>
#include <optional>
#include <span>
#include <cstring>

#include <poll.h>

namespace net {

inline constexpr uint16_t DEFAULT_PORT = 8080;
inline constexpr std::string_view LOCALHOST = "127.0.0.1";
inline constexpr auto PING_INTERVAL = std::chrono::seconds(2);

class Socket {
public:
    Socket() : fd_(socket(AF_INET, SOCK_STREAM, 0)) {
        if (fd_ < 0) throw std::runtime_error("Failed to create socket");
    }

    explicit Socket(int fd) : fd_(fd) {
        if (fd_ < 0) throw std::runtime_error("Invalid socket fd");
    }

    ~Socket() { if (fd_ >= 0) close(fd_); }

    Socket(Socket&& other) noexcept : fd_(std::exchange(other.fd_, -1)) {}
    Socket& operator=(Socket&& other) noexcept {
        if (this != &other) {
            if (fd_ >= 0) close(fd_);
            fd_ = std::exchange(other.fd_, -1);
        }
        return *this;
    }
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    [[nodiscard]] int fd() const noexcept { return fd_; }

    void send(std::string_view data) const {
        if (write(fd_, data.data(), data.size()) < 0)
            throw std::runtime_error("Send failed");
    }

    [[nodiscard]] std::optional<std::string> receive(size_t max_size = 256, int timeout_ms = 100) const {
        pollfd pfd{fd_, POLLIN, 0};
        if (poll(&pfd, 1, timeout_ms) <= 0) return std::nullopt;
        std::string buffer(max_size, '\0');
        ssize_t n = read(fd_, buffer.data(), max_size);
        if (n <= 0) return std::nullopt;
        buffer.resize(n);
        return buffer;
    }

private:
    int fd_ = -1;
};

class Address {
public:
    Address(std::string_view host, uint16_t port) {
        std::memset(&addr_, 0, sizeof(addr_));
        addr_.sin_family = AF_INET;
        addr_.sin_port = htons(port);
        addr_.sin_addr.s_addr = inet_addr(std::string(host).c_str());
    }

    explicit Address(uint16_t port) {
        std::memset(&addr_, 0, sizeof(addr_));
        addr_.sin_family = AF_INET;
        addr_.sin_port = htons(port);
        addr_.sin_addr.s_addr = INADDR_ANY;
    }

    [[nodiscard]] const sockaddr* ptr() const noexcept {
        return reinterpret_cast<const sockaddr*>(&addr_);
    }

    [[nodiscard]] socklen_t size() const noexcept { return sizeof(addr_); }

private:
    sockaddr_in addr_{};
};

class Server {
public:
    explicit Server(uint16_t port) : addr_(port) {
        int opt = 1;
        setsockopt(socket_.fd(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        
        if (bind(socket_.fd(), addr_.ptr(), addr_.size()) < 0)
            throw std::runtime_error("Bind failed");
        if (listen(socket_.fd(), SOMAXCONN) < 0)
            throw std::runtime_error("Listen failed");
    }

    [[nodiscard]] Socket accept() const {
        int client_fd = ::accept(socket_.fd(), nullptr, nullptr);
        return Socket(client_fd);
    }

    [[nodiscard]] std::optional<Socket> try_accept(int timeout_ms = 100) const {
        pollfd pfd{socket_.fd(), POLLIN, 0};
        if (poll(&pfd, 1, timeout_ms) <= 0) return std::nullopt;
        int client_fd = ::accept(socket_.fd(), nullptr, nullptr);
        if (client_fd < 0) return std::nullopt;
        return Socket(client_fd);
    }

private:
    Socket socket_;
    Address addr_;
};

[[nodiscard]] inline Socket connect(std::string_view host, uint16_t port) {
    Socket socket;
    Address addr(host, port);
    if (::connect(socket.fd(), addr.ptr(), addr.size()) < 0)
        throw std::runtime_error("Connection failed");
    return socket;
}

} // namespace net
