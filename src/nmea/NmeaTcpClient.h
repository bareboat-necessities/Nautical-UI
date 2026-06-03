#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>

namespace helm::nmea {

struct TcpEndpoint {
    std::string host;
    std::uint16_t port{0};

    [[nodiscard]] std::string display() const;
};

bool parse_tcp_nmea0183_uri(std::string_view uri, TcpEndpoint& out, std::string& error);

class NmeaTcpClient {
public:
    using LineCallback = std::function<void(std::string)>;
    using StatusCallback = std::function<void(bool connected, std::string message)>;

    NmeaTcpClient() = default;
    ~NmeaTcpClient();

    NmeaTcpClient(const NmeaTcpClient&) = delete;
    NmeaTcpClient& operator=(const NmeaTcpClient&) = delete;

    void set_line_callback(LineCallback callback);
    void set_status_callback(StatusCallback callback);

    void start(TcpEndpoint endpoint);
    void stop();

    [[nodiscard]] bool running() const noexcept;
    [[nodiscard]] TcpEndpoint endpoint() const;

private:
    void run();
    void emit_status(bool connected, std::string message);
    void emit_line(std::string line);

    mutable std::mutex callback_mutex_;
    LineCallback line_callback_;
    StatusCallback status_callback_;

    std::atomic_bool stop_requested_{false};
    std::atomic_bool running_{false};
    TcpEndpoint endpoint_{};
    std::thread thread_;
};

} // namespace helm::nmea
