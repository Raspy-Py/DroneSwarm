#ifndef WORKER_HPP
#define WORKER_HPP

#include "network_module.hpp"
#include "telemetry_reader.hpp"
#include <asio.hpp>

class Worker {
public:
    Worker(std::string& network_device, std::string& telemetry_device);
    void run();

private:
    asio::io_service io;
    NetworkReader network_reader;
    TelemetryReader telemetry_reader;
};

#endif // WORKER_HPP
