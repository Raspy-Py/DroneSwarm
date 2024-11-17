cmake_minimum_required(VERSION 2.8...3.13)

if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Build type not defined. Set 'CMAKE_BUILD_TYPE=Release' force ")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Set Release force " FORCE)
endif()

project(media C CXX)


##################################################
# PACKAGES
##################################################

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(PkgConfig)

find_package(RkAiq REQUIRED)
add_definitions(-DRKAIQ)

find_package(EasyMedia REQUIRED)


##################################################
# LIBS
##################################################

# librtsp
set(RTSP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/rtsp/include)
set(RTSP_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/libs/rtsp/lib/librtsp.a)

#libdetection
set(LED_LIB ${CMAKE_CURRENT_SOURCE_DIR}/libs/led/lib/libdetection.a)
set(LED_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/led/include)

#eigen
set(EIGEN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/eigen/include)


##################################################
# BUILD
##################################################

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

include_directories(
        ${RTSP_INCLUDE_DIR}
        ${RKAIQ_INCLUDE_DIRS}
        ${EASYMEDIA_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/common
        ${LED_INCLUDE_DIR}
        ${EIGEN_INCLUDE_DIR}
)

file(GLOB_RECURSE COMMON_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/common/*.cpp)

# 1. video_capture 
add_executable(video_capture
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vi_venc_jpeg.cpp
        ${COMMON_SOURCES}
)
target_link_libraries(video_capture
        ${EASYMEDIA_LIBRARY}
        ${RKAIQ_LIBRARY}
        pthread
)

# 2. video_stream
add_executable(video_stream
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vi_venc_h264.cpp
        ${COMMON_SOURCES}
)
target_link_libraries(video_stream
        ${EASYMEDIA_LIBRARY}
        ${RKAIQ_LIBRARY}
        ${RTSP_LIBRARY}
        pthread
)

# 3. read_and_convert
add_executable(read_and_convert
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vi_rga_bin.cpp
        ${COMMON_SOURCES}
)
target_link_libraries(read_and_convert
        ${LED_LIB}
        ${EASYMEDIA_LIBRARY}
        ${RKAIQ_LIBRARY}
        pthread
)

##################################################
# INSTALL
##################################################

install(
        TARGETS video_capture video_stream
        DESTINATION ${CMAKE_INSTALL_PREFIX}
)
