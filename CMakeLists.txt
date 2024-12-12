cmake_minimum_required(VERSION 2.8...3.13)

project(droneswarm C CXX)

# Dependencies
include(FetchContent)
include(cmake/asio.cmake)
include(cmake/msp.cmake)
include(cmake/eigen.cmake)

# Native dependencies
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")


# Main drone swarm library
# add_subdirectory(src/telemetry)
# add_subdirectory(src/network)
add_subdirectory(src/media)
# add_subdirectory(src/control)
add_subdirectory(src/slam)
#add_subdirectory(src/ml_tools)
