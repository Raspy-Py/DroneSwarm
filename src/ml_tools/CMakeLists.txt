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


##################################################
# LIBS
##################################################


# rknn api
set(RKNN_API_LIBRARY_PATH ${CMAKE_CURRENT_SOURCE_DIR}/libs/rknn_api)
set(RKNN_API_INCLUDE_DIR ${RKNN_API_LIBRARY_PATH}/include)
set(RKNN_API_LIBRARY ${RKNN_API_LIBRARY_PATH}/lib/librknn_api.so)

# stb
set(STB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/stb/include)


##################################################
# BUILD
##################################################

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/common
        ${RKNN_API_INCLUDE_DIR}
        ${STB_INCLUDE_DIR}
)

file(GLOB_RECURSE COMMON_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/common/*.cpp)

# 1. rknn_api_version
add_executable(rknn_api_version
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rknn_api_version.cpp
)

target_link_libraries(rknn_api_version
        ${RKNN_API_LIBRARY}
        pthread
        dl
)

# 2. alike_infer_test
add_executable(alike_infer_test
        ${CMAKE_CURRENT_SOURCE_DIR}/src/alike_infer_test.cpp
        ${COMMON_SOURCES}
)

target_link_libraries(alike_infer_test
        ${RKNN_API_LIBRARY}
        pthread
        dl
)

##################################################
# INSTALL
##################################################

install(
        TARGETS rknn_api_version alike_infer_test
        DESTINATION ${CMAKE_INSTALL_PREFIX}
)
