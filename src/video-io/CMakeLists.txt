cmake_minimum_required(VERSION 2.8...3.13)

project(video-io C CXX)

set(CMAKE_CXX_STANDARD 11)

add_executable(${PROJECT_NAME} main.cpp)

install(
        TARGETS ${PROJECT_NAME} 
        DESTINATION ${CMAKE_INSTALL_PREFIX}
)
