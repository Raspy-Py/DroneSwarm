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
#include <iostream>


#include "sample_common.h"
#include <easymedia/rkmedia_api.h>
#include <easymedia/rkmedia_vdec.h>
#include <easymedia/rkmedia_rga.h>

#include <rtsp_demo.h>
#include <led_detection.hpp>


const char* DEVICE_NAME = "rkispp_scale0";
const char* IQ_FILE_DIR = "/etc/iqfiles";
const char* STREAM_ADDRESS = "/live/main_stream";

char OUTPUT_FILE[] = "/tmp/output.rgb";

RK_U32 STREAM_PORT = 554;
RK_S32 VI_CHANNEL = 0;
RK_S32 CAMERA_ID = 0;
RK_S32 RGA_CHANNEL = 0;
RK_S32 RGA_DEVICE_ID = 0;

RK_U32 IMAGE_WIDTH = 512;
RK_U32 IMAGE_HEIGHT = 288;

int AUTO_EXPOSURE_FRAMES = 60;

typedef struct {
  char *filePath;
  int frameCount;
} OutputArgs;

static bool quit = false;
static void sigtermHandler(int sig) {
  quit = true;
}

static LedDetector ld;
static void *GetConvertedFrame(void *arg) {
  OutputArgs *outArgs = (OutputArgs *)arg;

  MEDIA_BUFFER mediaBuffer = NULL;

  while (!quit) {
    mediaBuffer = RK_MPI_SYS_GetMediaBuffer(RK_ID_RGA, 0, -1);
    if (!mediaBuffer) break;
    void *data = RK_MPI_MB_GetPtr(mediaBuffer);
    int width = IMAGE_WIDTH;
    int height = IMAGE_HEIGHT;
    int channels = 3;
    ld.map_test(data, width, height);
    printf("\n\n\n\n");

    RK_MPI_MB_ReleaseBuffer(mediaBuffer);
  }

  return NULL;
}

int main(int argc, char *argv[]) {
  RK_U32 input_width = IMAGE_WIDTH;
  RK_U32 input_height = IMAGE_HEIGHT;

  if (argc > 1)
      input_width = atoi(argv[1]);
  if (argc > 2)
      input_height = atoi(argv[2]);

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
  signal(SIGINT, sigtermHandler);
  
  RK_MPI_SYS_Init();
  
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
                      //  image format,      x, y, width, height, hor. stride, vert. stride
  RGA_ATTR_S rgaAttr = { {IMAGE_TYPE_NV12,   0, 0, input_width,  input_height,   input_width,        input_height},
                         {IMAGE_TYPE_RGB888, 0, 0, input_width,  input_height,   input_width,        input_height},
                      // rotation,    buffer pool, buffer pool count, flip
                         0,           RK_TRUE,     2,                 RGA_FLIP_NULL };

  RK_MPI_RGA_CreateChn(RGA_CHANNEL, &rgaAttr);

  MPP_CHN_S srcChn = { RK_ID_VI, CAMERA_ID, VI_CHANNEL }, destChn = { RK_ID_RGA, RGA_DEVICE_ID, RGA_CHANNEL };
  RK_MPI_SYS_Bind(&srcChn, &destChn);

  OutputArgs outArgs = { OUTPUT_FILE, AUTO_EXPOSURE_FRAMES };
  pthread_t thread;
  pthread_create(&thread, NULL, GetConvertedFrame, &outArgs);

  pthread_join(thread, NULL);
  RK_MPI_RGA_DestroyChn(0);
  RK_MPI_VI_DisableChn(0, 0);
  return 0;
}
