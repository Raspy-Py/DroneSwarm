#ifndef MASTER_HPP
#define MASTER_HPP

#include "network_module.hpp"
#include "telemetry_reader.hpp"
#include <asio.hpp>

class Master {
public:
    Master(std::string& network_device, std::string& telemetry_device);
    void run();

private:
    asio::io_service io;
    NetworkWriter network_writer;
    TelemetryReader telemetry_reader;
};

#endif // MASTER_HPP
