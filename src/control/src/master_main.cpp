#include "master.hpp"
#include <thread>

Master::Master(std::string& network_device, std::string& telemetry_device)
    : io{}, network_writer{io, network_device}, telemetry_reader{telemetry_device} {}

void Master::run() {
    std::thread telemetry_thread(&TelemetryReader::loop, &telemetry_reader);
    telemetry_thread.detach();
    network_writer.async_write("Initial message", 15);
    while (true)
    {
        io.run_one();
        if (network_writer.write_complete) {
            network_writer.write_complete = false;
            network_writer.async_write(&telemetry_reader.get_data(), sizeof(TelemetryData));
        }        
    }
}

int main(int argc, char const *argv[]) {
    std::string network_device = argc > 1 ? argv[1] : "/dev/ttyS0";
    std::string telemetry_device = argc > 2 ? argv[2] : "/dev/ttyS3";
    Master master{network_device, telemetry_device};
    master.run();
    return 0;
}
