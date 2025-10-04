#pragma once
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

class Socket {
protected:
    int fd;
public:
    Socket() : fd(-1) {}
    explicit Socket(int fd_) : fd(fd_) {}
    virtual ~Socket() {
        if (fd != -1) close(fd);
    }

    // 禁止拷贝，允许移动
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket(Socket&& other) noexcept : fd(other.fd) {
        other.fd = -1;
    }
    Socket& operator=(Socket&& other) noexcept {
        if (this != &other) {
            if (fd != -1) close(fd);
            fd = other.fd;
            other.fd = -1;
        }
        return *this;
    }

    ssize_t send(const void* buf, size_t len) {
        return ::send(fd, buf, len, 0);
    }

    ssize_t recv(void* buf, size_t len) {
        return ::recv(fd, buf, len, 0);
    }

    int getFd() const { return fd; }
};

class TcpClient : public Socket {
public:
    TcpClient() {
        fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd == -1) throw std::runtime_error("socket() failed");
    }

    void connectTo(const std::string& ip, uint16_t port) {
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        if (::inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) {
            throw std::runtime_error("inet_pton() failed");
        }
        if (::connect(fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
            throw std::runtime_error("connect() failed");
        }
    }
};

class TcpServer : public Socket {
public:
    TcpServer() {
        fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd == -1) throw std::runtime_error("socket() failed");
    }

    void bindAndListen(uint16_t port, int backlog = 5) {
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        int opt = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        if (::bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0)
            throw std::runtime_error("bind() failed");
        if (::listen(fd, backlog) < 0)
            throw std::runtime_error("listen() failed");
    }

    Socket acceptClient() {
        int clientFd = ::accept(fd, nullptr, nullptr);
        if (clientFd < 0) throw std::runtime_error("accept() failed");
        return Socket(clientFd); // 用 Socket 封装客户端
    }
};
