cmake_minimum_required(VERSION 2.8...3.13)

project(droneswarm C CXX)

# Dependencies
include(FetchContent)
include(externals/asio.cmake)
include(externals/msp.cmake)
# include(externals/eigen.cmake)

# Main drone swarm library
add_subdirectory(src/telemetry)
add_subdirectory(src/network)
add_subdirectory(src/control)
