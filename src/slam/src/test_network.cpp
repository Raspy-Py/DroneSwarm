// std
#include <getopt.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <vector>
#include <iostream>
#include <thread>
#include <atomic>
#include <cstring>

#include <asio.hpp>

#include <rknn_api.h>
#include <rtsp_demo.h>
#include "rknn_utils.h"

#include "thread_safe_queue.h"

/*
0. Load RKNN model
 # in a loop:
1. Get image from camera
2. Convert image from NV12 to RGB [DONE in RGA]
3. Quantize the image
4. Transpose the image from HWC to CHW [CAN BE PERFORMED BY DRIVER]
5. Run the model
6. Get the output
*/

static std::atomic_bool quit{false};
static void sigterm_handler(int sig) {
  quit = true;
}


class Broadcaster {
    const uint32_t BAUD_RATE = 921600;

public:
    Broadcaster(std::string port)
        : m_port(port), m_io()
        , m_serial_port(m_io, m_port)
    {
        m_serial_port.set_option(asio::serial_port::baud_rate(BAUD_RATE));
    }

    void run_sync() {
        while (!quit) {
            m_asio_buffer = {1, 2, 3, 4, 5};
            uint32_t transfered = asio::write(m_serial_port, asio::buffer(m_asio_buffer));
            std::cout << "================> [WRITTEN " << transfered << " ] <==============" << std::endl;
        }
    }

private:
    std::string m_port;
    asio::io_service m_io;
    asio::serial_port m_serial_port;
    std::vector<uint8_t> m_asio_buffer;
};


int main(int argc, char **argv)
{
    signal(SIGINT, sigterm_handler);

    std::string port = "/dev/ttyS0";
    Broadcaster broadcaster(port);
    broadcaster.run_sync();

    return 0;
}
