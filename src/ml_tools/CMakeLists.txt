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

# eigen 
set(EIGEN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/eigen/include)

# rknn api
set(RKNN_API_LIBRARY_PATH ${CMAKE_CURRENT_SOURCE_DIR}/libs/rknn_api)
set(RKNN_API_INCLUDE_DIR ${RKNN_API_LIBRARY_PATH}/include)
set(RKNN_API_LIBRARY ${RKNN_API_LIBRARY_PATH}/lib/librknn_api.so)

# stb
set(STB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/stb/include)

# librtsp
set(RTSP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/rtsp/include)
set(RTSP_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/libs/rtsp/lib/librtsp.a)


##################################################
# BUILD
##################################################

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/src/common
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${EASYMEDIA_INCLUDE_DIR}
        ${RKAIQ_INCLUDE_DIRS}
        ${RTSP_INCLUDE_DIR}

        ${RKNN_API_INCLUDE_DIR}
        ${EIGEN_INCLUDE_DIR}
        ${STB_INCLUDE_DIR}
)

file(GLOB COMMON_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/common/*.cpp)
file(GLOB ALIKE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/alike/*.cpp)

# 1. rknn_api_version
add_executable(rknn_api_version
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rknn_api_version.cpp
)

target_link_libraries(rknn_api_version
        ${EASYMEDIA_LIBRARY}
        ${RKAIQ_LIBRARY}
        ${RKNN_API_LIBRARY}
        pthread
)
# 2. alike_infer_test
add_executable(alike_infer_test
        ${CMAKE_CURRENT_SOURCE_DIR}/src/alike_infer_test.cpp
        ${COMMON_SOURCES}
)

target_link_libraries(alike_infer_test
        ${EASYMEDIA_LIBRARY}
        ${RKAIQ_LIBRARY}
        ${RKNN_API_LIBRARY}
        pthread
        dl
)

# 3. eigen_test
add_executable(eigen_test
        ${CMAKE_CURRENT_SOURCE_DIR}/src/eigen_test.cpp
)

target_link_libraries(eigen_test
        ${EASYMEDIA_LIBRARY}
        ${RKAIQ_LIBRARY}
        pthread
)

# 4. detect_keypoints
add_executable(detect_keypoints
        ${CMAKE_CURRENT_SOURCE_DIR}/src/detect_keypoints.cpp
        ${COMMON_SOURCES}
        ${ALIKE_SOURCES}
)

target_link_libraries(detect_keypoints
        ${EASYMEDIA_LIBRARY}
        ${RKAIQ_LIBRARY}
        ${RKNN_API_LIBRARY}
        ${RTSP_LIBRARY}
        pthread
        dl
)


##################################################
# INSTALL
##################################################

install(
        TARGETS rknn_api_version alike_infer_test eigen_test detect_keypoints
        DESTINATION ${CMAKE_INSTALL_PREFIX}
)
