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

#include <rtsp_demo.h>


const char* DEVICE_NAME = "rkispp_scale0";
const char* IQ_FILE_DIR = "/etc/iqfiles";

RK_S32 VI_CHANNEL = 0;
RK_S32 VENC_CHANNEL = 0;
RK_S32 CAMERA_ID = 0;

rtsp_demo_handle g_rtsplive = NULL;
static rtsp_session_handle g_rtsp_session;
static bool quit = false;

// Video encoder callback
void venc_callback(MEDIA_BUFFER mb);

static void sigterm_handler(int sig);

int main(int argc, char** argv){
    RK_U32 input_width = 1920;
    RK_U32 input_height = 1080;
    IMAGE_TYPE_E input_pix_fmt = IMAGE_TYPE_NV12;
    RK_U32 input_buf_cnt = 3;
    RK_U32 input_frame_rate = 30;
    RK_BOOL fec_enable = RK_FALSE;
    rk_aiq_working_mode_t hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;
    CODEC_TYPE_E codec_type = RK_CODEC_TYPE_H264;

    RK_MPI_SYS_Init();

    // Initialize Image Signal Processor
    SAMPLE_COMM_ISP_Init(CAMERA_ID, hdr_mode, fec_enable, IQ_FILE_DIR);
    SAMPLE_COMM_ISP_Run(CAMERA_ID);
    SAMPLE_COMM_ISP_SetFrameRate(CAMERA_ID, input_frame_rate);

    // Initialize RTSP server
    g_rtsplive = create_rtsp_demo(554);
    g_rtsp_session = rtsp_new_session(g_rtsplive, "/live/main_stream");
    rtsp_set_video(g_rtsp_session, RTSP_CODEC_ID_VIDEO_H264, NULL, 0);


    // Create a video input channel
    VI_CHN_ATTR_S vi_chn_attr;
    vi_chn_attr.pcVideoNode = DEVICE_NAME;
    vi_chn_attr.u32BufCnt = input_buf_cnt;
    vi_chn_attr.u32Width = input_width;
    vi_chn_attr.u32Height = input_height;
    vi_chn_attr.enPixFmt = input_pix_fmt;
    vi_chn_attr.enWorkMode = VI_WORK_MODE_NORMAL;
    vi_chn_attr.enBufType = VI_CHN_BUF_TYPE_MMAP;
    if(RK_MPI_VI_SetChnAttr(CAMERA_ID, VI_CHANNEL, &vi_chn_attr) != 0){
        printf("Create VI[0] failed! \n");
        return -1;
    }
    if(RK_MPI_VI_EnableChn(CAMERA_ID, VI_CHANNEL)){
        printf("Enable VI[0] failed!\n");
        return -1;
    }

    // Create a video encoder
    VENC_CHN_ATTR_S venc_chn_attr{};
    venc_chn_attr.stVencAttr.imageType = input_pix_fmt;
    venc_chn_attr.stVencAttr.u32PicWidth = input_width;
    venc_chn_attr.stVencAttr.u32PicHeight = input_height;
    venc_chn_attr.stVencAttr.u32VirWidth = input_width;
    venc_chn_attr.stVencAttr.u32VirHeight = input_height;

    venc_chn_attr.stVencAttr.enType = codec_type;
    venc_chn_attr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
    venc_chn_attr.stRcAttr.stH264Cbr.u32Gop = 30;
    venc_chn_attr.stRcAttr.stH264Cbr.u32BitRate = input_width * input_height;
    // frame rate: in 30/1, out 30/1.
    venc_chn_attr.stRcAttr.stH264Cbr.fr32DstFrameRateDen = 1;
    venc_chn_attr.stRcAttr.stH264Cbr.fr32DstFrameRateNum = 30;
    venc_chn_attr.stRcAttr.stH264Cbr.u32SrcFrameRateDen = 1;
    venc_chn_attr.stRcAttr.stH264Cbr.u32SrcFrameRateNum = 30;
    venc_chn_attr.stVencAttr.u32Profile = 77;

    if (RK_MPI_VENC_CreateChn(VENC_CHANNEL, &venc_chn_attr) != 0){
        printf("Create Venc failed! \n");
        return -1;
    }

    // Set video encoder callback
    MPP_CHN_S stEncChn;
    stEncChn.enModId = RK_ID_VENC;
    stEncChn.s32ChnId = VENC_CHANNEL;
    if (RK_MPI_SYS_RegisterOutCb(&stEncChn, venc_callback)) {
        printf("Register Output callback failed!\n");
        return -1;
    }
    

    MPP_CHN_S src_channel{};
    src_channel.enModId = RK_ID_VI;
    src_channel.s32ChnId = VI_CHANNEL;
    MPP_CHN_S dst_channel{};
    dst_channel.enModId = RK_ID_VENC;
    dst_channel.s32ChnId = VENC_CHANNEL;
    if (RK_MPI_SYS_Bind(&src_channel, &dst_channel)) {
        printf("Bind VI[%d] to VENC[%d]::H264 failed!\n", VI_CHANNEL, VENC_CHANNEL);
        return -1;
    }


    // Now it will continuously receive frames without user interaction
    signal(SIGINT, sigterm_handler);
    while (!quit) {
        usleep(1000); // Adjust this sleep as needed for frame pacing
    }

    // Unbind VI and VENC
    RK_MPI_SYS_UnBind(&src_channel, &dst_channel);

    // Clean-up in reverse order
    RK_MPI_VENC_DestroyChn(VENC_CHANNEL);
    RK_MPI_VI_DisableChn(CAMERA_ID, VI_CHANNEL);
    SAMPLE_COMM_ISP_Stop(CAMERA_ID);    

    return 0;
}

void venc_callback(MEDIA_BUFFER mb) {
  static RK_S32 packet_cnt = 0;
  if (quit)
    return;

  printf("#Get packet %d, size %zu\n", packet_cnt, RK_MPI_MB_GetSize(mb));

  if (g_rtsplive && g_rtsp_session) {
    rtsp_tx_video(g_rtsp_session, 
        reinterpret_cast<uint8_t*>(RK_MPI_MB_GetPtr(mb)), 
        RK_MPI_MB_GetSize(mb), RK_MPI_MB_GetTimestamp(mb)
    );
    rtsp_do_event(g_rtsplive);
  }

  RK_MPI_MB_ReleaseBuffer(mb);
  packet_cnt++;

}

static void sigterm_handler(int sig){
    printf("Quiting...");
    quit = true;
}
