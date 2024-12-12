FetchContent_Declare(
    msp
    GIT_REPOSITORY  https://github.com/christianrauch/msp.git
    GIT_TAG         59e4742e5f66009f7f1af167de9f214b93a7a9c4 # v3.3.0
)

FetchContent_Populate(msp)
set(MSP_SOURCE_DIR ${msp_SOURCE_DIR}/src)
set(MSP_INCLUDE_DIR ${msp_SOURCE_DIR}/inc/msp)

# client library
add_library(mspclient ${MSP_SOURCE_DIR}/Client.cpp ${MSP_SOURCE_DIR}/PeriodicTimer.cpp)
target_link_libraries(mspclient asio)
target_include_directories(mspclient PUBLIC ${MSP_INCLUDE_DIR})

# high-level API
add_library(msp_fcu ${MSP_SOURCE_DIR}/FlightController.cpp)
target_link_libraries(msp_fcu mspclient)
target_include_directories(msp_fcu PUBLIC ${MSP_INCLUDE_DIR})
