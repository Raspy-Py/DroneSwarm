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

#include "sample_common.h"
#include <easymedia/rkmedia_api.h>
#include <easymedia/rkmedia_vdec.h>

class Camera {
public:
    Camera();
    ~Camera();

private:
    RK_S32 m_camera_id;

    static const RK_S32 m_vi_channel;

};
