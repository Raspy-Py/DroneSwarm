#include <iostream>
#include <chrono>
#include <cmath>
#include <asio.hpp>

int main(int argc, char *argv[]) {
    asio::io_service io;
    size_t size = 10 * 1024; // 10 KB of test data
    unsigned int num_writes = 20;

    std::string port = "/dev/ttyS0";
    unsigned int baud_rate = argc > 1 ? std::stoi(argv[1]) : 921600; // Maximal baud rate 

    asio::serial_port s_port(io, port);
    s_port.set_option(asio::serial_port::baud_rate(baud_rate));

    std::string data(size, '\0');
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;

    double total_time = 0;
    double total_squared_time = 0;

    printf("Starting benchmark with baud rate %d\n", baud_rate);
    asio::write(s_port, asio::buffer(data, size));
    printf("First write done. Starting benchmark\n");

    for (size_t i = 0; i < num_writes; i++)
    {
        start = std::chrono::high_resolution_clock::now();
        asio::write(s_port, asio::buffer(data, size));
        end = std::chrono::high_resolution_clock::now();
        auto delta_t = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        total_time += delta_t;
        total_squared_time += delta_t * delta_t;
    }

    auto mean_time = total_time / num_writes;
    auto std_dev = sqrt(total_squared_time / num_writes - mean_time * mean_time);
    std::cout << "Packet size: " << size/1024 << " KB\n";
    std::cout << "Average sending time: " << mean_time << " milliseconds\n";
    std::cout << "Standard deviation: " << std_dev << " milliseconds\n";
    
    return 0;
}
