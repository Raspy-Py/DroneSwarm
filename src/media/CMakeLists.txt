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
        ${EASYMEDIA_LIBRARY}
        ${RKAIQ_LIBRARY}
        pthread
)

##################################################
# INSTALL
##################################################

install(
        TARGETS video_capture video_stream read_and_convert
        DESTINATION ${CMAKE_INSTALL_PREFIX}
)
