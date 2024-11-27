set(NETWORK_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(NETWORK_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# network module
add_library(network_module ${NETWORK_SOURCE_DIR}/network_module.cpp)
target_link_libraries(network_module asio)
target_include_directories(network_module PUBLIC ${NETWORK_INCLUDE_DIR})

# network benchmark
add_executable(network_benchmark ${NETWORK_SOURCE_DIR}/network_benchmark.cpp)
target_link_libraries(network_benchmark asio)
target_include_directories(network_benchmark PUBLIC ${NETWORK_INCLUDE_DIR})

# install
install(TARGETS network_benchmark
    RUNTIME DESTINATION .)
