#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <thread>
#include <functional>

#include "sample_common.h"
#include <easymedia/rkmedia_api.h>
#include <easymedia/rkmedia_vdec.h>

#include "core.hpp"

class Camera {
public:
    Camera(RK_S32 camera_id, RK_S32 vi_channel, RK_U32 width, RK_U32 height);
    ~Camera();

    void Open(RK_S32 camera_id, RK_S32 vi_channel, RK_U32 width, RK_U32 height);

private:
    // Configures camera input to be used as raw data source
    void SetCameraCallback(std::function<void(MEDIA_BUFFER)>&& callback);

    // Redirects output to a different RK defice (RGA, VENC, etc)
    void RedirectOuput(const RKModule& module);

private:
    // General device attributes
    RK_S32 m_camera_id = 0;
    RK_S32 m_vi_channel = 0;
    
    // Video input attributes
    RK_U32 m_width = 1920;
    RK_U32 m_height = 1080;
    RK_U32 m_channels = 3;
    RK_U32 input_frame_rate = 30;
    RK_BOOL m_fec_enable = RK_FALSE;
    IMAGE_TYPE_E m_pix_fmt = IMAGE_TYPE_NV12;
    rk_aiq_working_mode_t hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;

    // You can either use the pipeline or cd output callback
    RKPipeline m_pipeline;
    bool m_pipeline_enabled = false;

    // Callback
    std::function<void(MEDIA_BUFFER)> m_callback;
    std::thread m_thread;

    const char* DEVICE_NAME = "rkispp_scale0";
    const char* IQ_FILES_DIR = "/etc/iqfiles";
};
