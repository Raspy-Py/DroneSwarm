set(CONTROL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(CONTROL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# high level control for master
add_executable(master_main ${CONTROL_SOURCE_DIR}/master_main.cpp)
target_link_libraries(master_main asio network_module telemetry_module)
target_include_directories(master_main PUBLIC ${CONTROL_INCLUDE_DIR})

# high level control for worker
add_executable(worker_main ${CONTROL_SOURCE_DIR}/worker_main.cpp)
target_link_libraries(worker_main asio network_module telemetry_module)
target_include_directories(worker_main PUBLIC ${CONTROL_INCLUDE_DIR})

# install
install(TARGETS master_main worker_main
    RUNTIME DESTINATION .)
