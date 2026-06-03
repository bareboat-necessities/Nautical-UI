#include "nmea/NmeaTcpClient.h"

#include <array>
#include <cerrno>
#include <cstring>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

#ifdef _WIN32
    #ifndef NOMINMAX
    #define NOMINMAX
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <netdb.h>
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <unistd.h>
#endif

namespace helm::nmea {
namespace {

#ifdef _WIN32
using SocketHandle = SOCKET;
constexpr SocketHandle InvalidSocket = INVALID_SOCKET;

struct WsaSession {
    WsaSession() {
        WSADATA data{};
        if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
            throw std::runtime_error("WSAStartup failed");
        }
    }
    ~WsaSession() { WSACleanup(); }
};

void close_socket(SocketHandle s) {
    if (s != InvalidSocket) {
        closesocket(s);
    }
}

std::string last_socket_error() {
    return "socket error " + std::to_string(WSAGetLastError());
}
#else
using SocketHandle = int;
constexpr SocketHandle InvalidSocket = -1;

void close_socket(SocketHandle s) {
    if (s != InvalidSocket) {
        close(s);
    }
}

std::string last_socket_error() {
    return std::strerror(errno);
}
#endif

class SocketGuard {
public:
    explicit SocketGuard(SocketHandle s = InvalidSocket) : socket_(s) {}
    ~SocketGuard() { close_socket(socket_); }

    SocketGuard(const SocketGuard&) = delete;
    SocketGuard& operator=(const SocketGuard&) = delete;

    SocketHandle get() const noexcept { return socket_; }
    SocketHandle release() noexcept {
        SocketHandle s = socket_;
        socket_ = InvalidSocket;
        return s;
    }
    void reset(SocketHandle s = InvalidSocket) noexcept {
        close_socket(socket_);
        socket_ = s;
    }

private:
    SocketHandle socket_{InvalidSocket};
};

std::string trim_scheme(std::string_view uri) {
    constexpr std::string_view scheme = "tcp-nmea0183://";
    if (uri.rfind(scheme, 0) == 0) {
        uri.remove_prefix(scheme.size());
    }
    return std::string(uri);
}

bool parse_port(std::string_view s, std::uint16_t& port) {
    if (s.empty()) return false;
    unsigned long value = 0;
    for (char c : s) {
        if (c < '0' || c > '9') return false;
        value = value * 10UL + static_cast<unsigned long>(c - '0');
        if (value > 65535UL) return false;
    }
    if (value == 0UL) return false;
    port = static_cast<std::uint16_t>(value);
    return true;
}

SocketHandle connect_blocking(const TcpEndpoint& endpoint, std::string& error) {
    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    addrinfo* result = nullptr;
    const std::string port = std::to_string(endpoint.port);
    const int rc = getaddrinfo(endpoint.host.c_str(), port.c_str(), &hints, &result);
    if (rc != 0) {
#ifdef _WIN32
        error = "getaddrinfo failed: " + std::to_string(rc);
#else
        error = std::string("getaddrinfo failed: ") + gai_strerror(rc);
#endif
        return InvalidSocket;
    }

    SocketHandle connected = InvalidSocket;
    for (addrinfo* p = result; p != nullptr; p = p->ai_next) {
        SocketGuard s(::socket(p->ai_family, p->ai_socktype, p->ai_protocol));
        if (s.get() == InvalidSocket) {
            error = last_socket_error();
            continue;
        }

        if (::connect(s.get(), p->ai_addr, static_cast<int>(p->ai_addrlen)) == 0) {
            connected = s.release();
            break;
        }
        error = last_socket_error();
    }

    freeaddrinfo(result);
    return connected;
}

int recv_some(SocketHandle socket, char* buffer, int size) {
#ifdef _WIN32
    return ::recv(socket, buffer, size, 0);
#else
    return static_cast<int>(::recv(socket, buffer, static_cast<std::size_t>(size), 0));
#endif
}

} // namespace

std::string TcpEndpoint::display() const {
    return host + ":" + std::to_string(port);
}

bool parse_tcp_nmea0183_uri(std::string_view uri, TcpEndpoint& out, std::string& error) {
    const std::string body = trim_scheme(uri);
    if (body.empty()) {
        error = "empty endpoint";
        return false;
    }

    const auto colon = body.rfind(':');
    if (colon == std::string::npos) {
        error = "expected host:port";
        return false;
    }

    const std::string host = body.substr(0, colon);
    const std::string port_str = body.substr(colon + 1);
    if (host.empty()) {
        error = "empty host";
        return false;
    }

    std::uint16_t port = 0;
    if (!parse_port(port_str, port)) {
        error = "invalid TCP port";
        return false;
    }

    out.host = host;
    out.port = port;
    return true;
}

NmeaTcpClient::~NmeaTcpClient() {
    stop();
}

void NmeaTcpClient::set_line_callback(LineCallback callback) {
    std::scoped_lock lock(callback_mutex_);
    line_callback_ = std::move(callback);
}

void NmeaTcpClient::set_status_callback(StatusCallback callback) {
    std::scoped_lock lock(callback_mutex_);
    status_callback_ = std::move(callback);
}

void NmeaTcpClient::start(TcpEndpoint endpoint) {
    stop();
    endpoint_ = std::move(endpoint);
    stop_requested_ = false;
    running_ = true;
    thread_ = std::thread([this] { run(); });
}

void NmeaTcpClient::stop() {
    stop_requested_ = true;
    if (thread_.joinable()) {
        thread_.join();
    }
    running_ = false;
}

bool NmeaTcpClient::running() const noexcept {
    return running_;
}

TcpEndpoint NmeaTcpClient::endpoint() const {
    return endpoint_;
}

void NmeaTcpClient::emit_status(bool connected, std::string message) {
    StatusCallback cb;
    {
        std::scoped_lock lock(callback_mutex_);
        cb = status_callback_;
    }
    if (cb) {
        cb(connected, std::move(message));
    }
}

void NmeaTcpClient::emit_line(std::string line) {
    LineCallback cb;
    {
        std::scoped_lock lock(callback_mutex_);
        cb = line_callback_;
    }
    if (cb) {
        cb(std::move(line));
    }
}

void NmeaTcpClient::run() {
#ifdef _WIN32
    WsaSession wsa;
#endif
    while (!stop_requested_) {
        std::string error;
        emit_status(false, "connecting to " + endpoint_.display());
        SocketGuard socket(connect_blocking(endpoint_, error));
        if (socket.get() == InvalidSocket) {
            emit_status(false, "connect failed: " + error);
            for (int i = 0; i < 20 && !stop_requested_; ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            continue;
        }

        emit_status(true, "connected to " + endpoint_.display());
        std::string pending;
        pending.reserve(512);
        std::array<char, 1024> buffer{};

        while (!stop_requested_) {
            const int n = recv_some(socket.get(), buffer.data(), static_cast<int>(buffer.size()));
            if (n <= 0) {
                emit_status(false, n == 0 ? "remote closed" : "recv failed: " + last_socket_error());
                break;
            }

            pending.append(buffer.data(), static_cast<std::size_t>(n));
            for (;;) {
                const auto pos = pending.find('\n');
                if (pos == std::string::npos) {
                    if (pending.size() > 4096) {
                        pending.clear();
                    }
                    break;
                }

                std::string line = pending.substr(0, pos + 1);
                pending.erase(0, pos + 1);
                emit_line(std::move(line));
            }
        }
    }
    emit_status(false, "stopped");
    running_ = false;
}

} // namespace helm::nmea
