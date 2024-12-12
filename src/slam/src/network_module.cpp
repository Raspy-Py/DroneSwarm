#include "network_module.hpp"

NetworkReader::NetworkReader(asio::io_service& io, const std::string& device) : s_port{io, device} {
    s_port.set_option(asio::serial_port::baud_rate(MAX_BAUD_RATE));
}

NetworkWriter::NetworkWriter(asio::io_service& io, const std::string& device) : s_port{io, device} {
    s_port.set_option(asio::serial_port::baud_rate(MAX_BAUD_RATE));
}

void NetworkWriter::async_write(const void* data, size_t size) {
    asio::async_write(s_port, asio::buffer(data, size), [this](const asio::error_code& error, size_t bytes_transferred) {
        if (error) {
            std::cerr << "Error writing to serial port: " << error.message() << std::endl;
        }
        else {
            write_complete = true;
        }
    });
}

void NetworkReader::async_read(std::string& data) {
    asio::async_read_until(s_port, asio::dynamic_buffer(data), '\0', [this](const asio::error_code& error, size_t bytes_transferred) {
        if (error) {
            std::cerr << "Error reading from serial port: " << error.message() << std::endl;
        }
        else {
            read_complete = true;
        }
    });
}
