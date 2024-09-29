cmake_minimum_required(VERSION 2.8...3.13)

project(vision-full C CXX)

# Add your modules here
add_subdirectory(
    src/video-io
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
