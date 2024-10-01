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


const char* DEVICE_NAME = "rkispp_scale0";
const char* IQ_FILE_DIR = "/etc/iqfiles";
static bool quit = false;

RK_S32 VI_CHANNEL = 0;
RK_S32 VENC_CHANNEL = 0;
RK_S32 CAMERA_ID = 0;

// Video encoder callback
void venc_callback(MEDIA_BUFFER mb);

static void sigterm_handler(int sig);

int main(int argc, char** argv){
    RK_U32 input_width = 1920;
    RK_U32 input_height = 1080;
    RK_U32 output_width = 720;
    RK_U32 output_height = 480;
    IMAGE_TYPE_E input_pix_fmt = IMAGE_TYPE_NV12;
    RK_U32 input_buf_cnt = 3;
    RK_U32 input_frame_rate = 30;
    RK_BOOL fec_enable = RK_FALSE;
    rk_aiq_working_mode_t hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;

    RK_MPI_SYS_Init();

    // Initialize Image Signal Processor
    SAMPLE_COMM_ISP_Init(CAMERA_ID, hdr_mode, fec_enable, IQ_FILE_DIR);
    SAMPLE_COMM_ISP_Run(CAMERA_ID);
    SAMPLE_COMM_ISP_SetFrameRate(CAMERA_ID, input_frame_rate);

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
    venc_chn_attr.stVencAttr.enType = RK_CODEC_TYPE_JPEG;
    venc_chn_attr.stVencAttr.imageType = input_pix_fmt;
    venc_chn_attr.stVencAttr.u32PicWidth = input_width;
    venc_chn_attr.stVencAttr.u32PicHeight = input_height;
    venc_chn_attr.stVencAttr.u32VirWidth = input_width;
    venc_chn_attr.stVencAttr.u32VirHeight = input_height;
    venc_chn_attr.stVencAttr.stAttrJpege.u32ZoomWidth = output_width;
    venc_chn_attr.stVencAttr.stAttrJpege.u32ZoomHeight = output_height;
    venc_chn_attr.stVencAttr.stAttrJpege.u32ZoomVirWidth = output_width;
    venc_chn_attr.stVencAttr.stAttrJpege.u32ZoomVirHeight = output_height;
    venc_chn_attr.stVencAttr.enRotation = VENC_ROTATION_0;
    venc_chn_attr.stVencAttr.stAttrJpege.bSupportDCF = RK_FALSE;

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
    
    
    // The encoder defaults to continuously receiving frames from the previous
    // stage. Before performing the bind operation, set s32RecvPicNum to 0 to
    // make the encoding enter the pause state.
    VENC_RECV_PIC_PARAM_S stRecvParam;
    stRecvParam.s32RecvPicNum = 0;
    RK_MPI_VENC_StartRecvFrame(0, &stRecvParam);

    MPP_CHN_S src_channel;
    src_channel.enModId = RK_ID_VI;
    src_channel.s32ChnId = VI_CHANNEL;
    MPP_CHN_S dst_channel;
    dst_channel.enModId = RK_ID_VENC;
    dst_channel.s32ChnId = VENC_CHANNEL;
    if (RK_MPI_SYS_Bind(&src_channel, &dst_channel)) {
        printf("Bind VI[%d] to VENC[%d]::JPEG failed!\n", VI_CHANNEL, VENC_CHANNEL);
        return -1;
    }

    int qfactor = 75; // Set a fixed qfactor value (or any other default value)
    VENC_JPEG_PARAM_S stJpegParam;
    stJpegParam.u32Qfactor = qfactor;
    RK_MPI_VENC_SetJpegParam(0, &stJpegParam); // Set JPEG params once

    VENC_CHN_PARAM_S venc_chn_param{};
    venc_chn_param.stCropCfg.bEnable = RK_FALSE;
    // venc_chn_param.stCropCfg.stRect.s32X = (qfactor / 2) * 2;
    // venc_chn_param.stCropCfg.stRect.s32Y = (qfactor / 2) * 2;
    // venc_chn_param.stCropCfg.stRect.u32Width = ((200 + qfactor) / 2) * 2;
    // venc_chn_param.stCropCfg.stRect.u32Height = ((200 + qfactor) / 2) * 2;
    venc_chn_param.stCropCfg.stRect.s32X = 0;
    venc_chn_param.stCropCfg.stRect.s32Y = 0;
    venc_chn_param.stCropCfg.stRect.u32Width = output_width;
    venc_chn_param.stCropCfg.stRect.u32Height = output_height;
    RK_MPI_VENC_SetChnParam(VENC_CHANNEL, &venc_chn_param); // Set channel params once

    // Start receiving frames continuously
    stRecvParam.s32RecvPicNum = -1;  // Set to -1 for continuous reception
    RK_MPI_VENC_StartRecvFrame(VENC_CHANNEL, &stRecvParam);

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
  static RK_U32 jpeg_id = 0;
//   printf("Get JPEG packet[%d]:ptr:%p, fd:%d, size:%zu, mode:%d, channel:%d, "
//          "timestamp:%lld\n",
//          jpeg_id, RK_MPI_MB_GetPtr(mb), RK_MPI_MB_GetFD(mb),
//          RK_MPI_MB_GetSize(mb), RK_MPI_MB_GetModeID(mb),
//          RK_MPI_MB_GetChannelID(mb), RK_MPI_MB_GetTimestamp(mb));

  char jpeg_path[128];
  sprintf(jpeg_path, "/tmp/video/frame%d.jpeg", jpeg_id);
  printf("Save JPEG packet[%d] to %s\n", jpeg_id, jpeg_path);
  FILE *file = fopen(jpeg_path, "w");
  if (file) {
    fwrite(RK_MPI_MB_GetPtr(mb), 1, RK_MPI_MB_GetSize(mb), file);
    fclose(file);
  }

  RK_MPI_MB_ReleaseBuffer(mb);
  jpeg_id++;
}

static void sigterm_handler(int sig){
    printf("Quiting...");
    quit = true;
}
