#include "telemetry_reader.hpp"

TelemetryData::TelemetryData(msp::FirmwareVariant fw_variant) : attitude{fw_variant}, imu{fw_variant} {}

TelemetryReader::TelemetryReader(std::string& device) : client{},
fw_variant{msp::FirmwareVariant::BAFL}, data{fw_variant} {
    client.setLoggingLevel(msp::client::LoggingLevel::WARNING);
    client.setVariant(fw_variant);
    client.start(device, MSP_BAUDRATE);
}

void TelemetryReader::read_imu() {
    if (client.sendMessage(data.imu) != 1) {
        std::cerr << "Failed to read IMU data" << std::endl;
    }
}

void TelemetryReader::read_attitude() {
    if (client.sendMessage(data.attitude) != 1) {
        std::cerr << "Failed to read attitude data" << std::endl;
    }
}

void TelemetryReader::loop() {
    while (true) {
        read_imu();
        read_attitude();
    }
}
