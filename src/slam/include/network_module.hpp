#ifndef NETWORK_MODULE_HPP
#define NETWORK_MODULE_HPP

#include <iostream>
#include <asio.hpp>

constexpr size_t MAX_BAUD_RATE = 921600;

class NetworkWriter {

public:
    NetworkWriter(asio::io_service& io, const std::string& device);

    void async_write(const void* data, size_t size);

    bool write_complete = false;
private:
    asio::serial_port s_port;
};

class NetworkReader {

public:
    NetworkReader(asio::io_service& io, const std::string& device);

    void async_read(std::string& data);
    bool read_complete = false;

private:
    asio::serial_port s_port;
};

#endif // NETWORK_MODULE_HPP
