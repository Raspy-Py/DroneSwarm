cmake_minimum_required(VERSION 2.8...3.13)

if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Build type not defined. Set 'CMAKE_BUILD_TYPE=Release' force ")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Set Release force " FORCE)
endif()

project(media C CXX)


##################################################
# PACKAGES
##################################################

message("CMAKE_MODULE_PATH: " ${CMAKE_MODULE_PATH})

find_package(PkgConfig)

# Rockchip's Automatic Image Quality Package
find_package(RkAiq REQUIRED)
add_definitions(-DRKAIQ)

# Easymedia wrapper for RkAIQ
find_package(EasyMedia REQUIRED)

##################################################
# LIBS
##################################################

# rknn api
set(RKNN_API_LIBRARY_PATH ${CMAKE_SOURCE_DIR}/libs/rknn_api)
set(RKNN_API_INCLUDE_DIR ${RKNN_API_LIBRARY_PATH}/include)
set(RKNN_API_LIBRARY ${RKNN_API_LIBRARY_PATH}/lib/librknn_api.so)

# stb
set(STB_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/libs/stb/include)

# librtsp
set(RTSP_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/libs/rtsp/include)
set(RTSP_LIBRARY ${CMAKE_SOURCE_DIR}/libs/rtsp/lib/librtsp.a)


##################################################
# BUILD
##################################################

# network module
# add_library(network_module ${NETWORK_SOURCE_DIR}/network_module.cpp)
# target_link_libraries(network_module asio)
# target_include_directories(network_module PUBLIC ${NETWORK_INCLUDE_DIR})

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

message("EIGEN_INCLUDE_DIR: " ${EIGEN_INCLUDE_DIR})

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/include/common
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${EASYMEDIA_INCLUDE_DIR}
        ${RKAIQ_INCLUDE_DIRS}
        ${RTSP_INCLUDE_DIR}

        ${RKNN_API_INCLUDE_DIR}
        ${EIGEN_INCLUDE_DIR}
        ${STB_INCLUDE_DIR}
)

file(GLOB COMMON_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/common/*.cpp)

# 1. slam_service
add_executable(slam_service
        ${CMAKE_CURRENT_SOURCE_DIR}/src/slam_service.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/network_module.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/dkd.cpp
        ${COMMON_SOURCES}
)

target_link_libraries(slam_service
        ${EASYMEDIA_LIBRARY}
        ${RKAIQ_LIBRARY}
        ${RKNN_API_LIBRARY}
        ${RTSP_LIBRARY}
        pthread
        asio
        dl
)


# 2. test_network
add_executable(test_network
        ${CMAKE_CURRENT_SOURCE_DIR}/src/test_network.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/network_module.cpp
)

target_link_libraries(test_network
        pthread
        asio
        dl
)

#target_link_libraries(slam_service PRIVATE asio)



##################################################
# INSTALL
##################################################

install(
        TARGETS slam_service test_network
        DESTINATION ${CMAKE_INSTALL_PREFIX}
)
