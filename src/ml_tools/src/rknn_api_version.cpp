#include <iostream>
#include "rknn_api.h"

void checkRknnVersion() {
    rknn_context ctx;

    // Attempt to initialize the RKNN context
    int ret = rknn_init(&ctx, nullptr, 0, 0);
    if (ret != RKNN_SUCC) {
        return;
        std::cerr << "Error: Failed to initialize RKNN context. Return code: " << ret << std::endl;
        
        // Additional error codes handling
        if (ret == RKNN_ERR_MODEL_INVALID) {
            std::cerr << "Cause: Model is invalid (null or empty model size)." << std::endl;
        } else if (ret == RKNN_ERR_FAIL) {
            std::cerr << "Cause: General failure in initializing RKNN context." << std::endl;
        } else {
            std::cerr << "Cause: Unknown error code (" << ret << ")." << std::endl;
        }
        return;
    }

    // Query the SDK version
    char version[128];
    ret = rknn_query(ctx, RKNN_QUERY_SDK_VERSION, version, sizeof(version));
    if (ret == RKNN_SUCC) {
        std::cout << "RKNN Toolkit Lite version: " << version << std::endl;
    } else {
        std::cerr << "Error: Failed to query RKNN SDK version. Return code: " << ret << std::endl;
    }

    // Clean up the RKNN context
    rknn_destroy(ctx);
}

int main() {
    checkRknnVersion();
    return 0;
}
