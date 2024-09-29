cmake_minimum_required(VERSION 2.8...3.13)

if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Build type not defined. Set 'CMAKE_BUILD_TYPE=Release' force ")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Set Release force " FORCE)
endif()

project(video-io C CXX)

include(cmake/rkaiq.cmake)
add_definitions(-DRKAIQ)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(${PROJECT_NAME} 
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/common/sample_common_isp.c
)

target_link_libraries(${PROJECT_NAME}
        easymedia
        ${RKAIQ_LIBRARY}
)

install(
        TARGETS ${PROJECT_NAME} 
        DESTINATION ${CMAKE_INSTALL_PREFIX}
)
