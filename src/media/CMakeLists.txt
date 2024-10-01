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

#if(USE_RKAIQ)
find_package(RkAiq REQUIRED)
include_directories(${RKAIQ_INCLUDE_DIRS})
add_definitions(-DRKAIQ)
#endif()

find_package(EasyMedia REQUIRED)

##################################################
# BUILD
##################################################

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

include_directories(
        ${RKAIQ_INCLUDE_DIRS}
        ${EASYMEDIA_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/common
)

file(GLOB_RECURSE COMMON_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/common/*.cpp)

# 1. media_test
add_executable(media_test
        ${CMAKE_CURRENT_SOURCE_DIR}/media_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/camera/camera.cpp
        ${COMMON_SOURCES}
)
target_link_libraries(media_test
        ${EASYMEDIA_LIBRARY}
        ${RKAIQ_LIBRARY}
        pthread
)

# 2. video_capture 
add_executable(video_capture
        ${CMAKE_CURRENT_SOURCE_DIR}/vi_venc_jpeg.cpp
        ${COMMON_SOURCES}
)
target_link_libraries(video_capture
        ${EASYMEDIA_LIBRARY}
        ${RKAIQ_LIBRARY}
        pthread
)

##################################################
# INSTALL
##################################################

install(
        TARGETS media_test video_capture
        DESTINATION ${CMAKE_INSTALL_PREFIX}
)
