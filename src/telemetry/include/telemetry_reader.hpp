#ifndef TELEMETRY_READER_HPP
#define TELEMETRY_READER_HPP

#include <iostream>
#include <Client.hpp>
#include <msp_msg.hpp>

constexpr size_t MSP_BAUDRATE = 115200;

struct TelemetryData {
    msp::msg::Attitude attitude;
    msp::msg::RawImu imu;

    TelemetryData(msp::FirmwareVariant fw_variant);
};

class TelemetryReader {

public:
    TelemetryReader(std::string& device);

    void read_imu();
    void read_attitude();
    void loop();
    const TelemetryData& get_data() const { return data; }

private:
    msp::client::Client client;
    msp::FirmwareVariant fw_variant;
    TelemetryData data;
};

#endif // TELEMETRY_READER_HPP
