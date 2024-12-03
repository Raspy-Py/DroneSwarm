set(TELEMETRY_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(TELEMETRY_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# telemetry library
add_library(telemetry_module ${TELEMETRY_SOURCE_DIR}/telemetry_reader.cpp)
target_link_libraries(telemetry_module mspclient msp_fcu)
target_include_directories(telemetry_module PUBLIC ${TELEMETRY_INCLUDE_DIR})
