FetchContent_Declare(
    asio
    URL      https://github.com/chriskohlhoff/asio/archive/asio-1-20-0.zip
)

find_package(Threads)
FetchContent_Populate(asio)

add_library(asio INTERFACE)
target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)
target_compile_definitions(asio INTERFACE ASIO_STANDALONE)
target_compile_features(asio INTERFACE cxx_std_11)
target_link_libraries(asio INTERFACE Threads::Threads)
