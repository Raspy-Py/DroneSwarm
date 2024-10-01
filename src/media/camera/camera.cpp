#include "camera/camera.hpp"

#include "sample_common.h"

static bool quit = false;

const RK_S32 Camera::m_vi_channel = 0;

static void *GetMediaBuffer(void *arg) {
  (void*)arg;
  const char *save_path = "/tmp/vi.yuv";
  int save_cnt = 1;
  int frame_id = 0;
  FILE *save_file = NULL;
  if (save_path) {
    save_file = fopen(save_path, "w");
    if (!save_file)
      printf("ERROR: Open %s failed!\n", save_path);
  }

  MEDIA_BUFFER mb = NULL;
  while (!quit) {
    mb = RK_MPI_SYS_GetMediaBuffer(RK_ID_VI, 0, -1);
    if (!mb) {
      printf("RK_MPI_SYS_GetMediaBuffer get null buffer!\n");
      break;
    }

    MB_IMAGE_INFO_S stImageInfo = {0};
    int ret = RK_MPI_MB_GetImageInfo(mb, &stImageInfo);
    if (ret)
      printf("Warn: Get image info failed! ret = %d\n", ret);

     printf("Get Frame:ptr:%p, fd:%d, size:%zu, mode:%d, channel:%d, "
           "timestamp:%lld, ImgInfo:<wxh %dx%d, fmt 0x%x>\n",
           RK_MPI_MB_GetPtr(mb), RK_MPI_MB_GetFD(mb), RK_MPI_MB_GetSize(mb),
           RK_MPI_MB_GetModeID(mb), RK_MPI_MB_GetChannelID(mb),
           RK_MPI_MB_GetTimestamp(mb), stImageInfo.u32Width,
           stImageInfo.u32Height, stImageInfo.enImgType);

    if (save_file) {
        RK_S32 image_size = RK_MPI_MB_GetSize(mb);
        printf("image_size: %d\n", image_size);
            
        fwrite(RK_MPI_MB_GetPtr(mb), 1, RK_MPI_MB_GetSize(mb), save_file);
        printf("#Save frame-%d to %s\n", frame_id++, save_path);
    }

    RK_MPI_MB_ReleaseBuffer(mb);

    if (save_cnt > 0)
      save_cnt--;

    // exit when complete
    if (!save_cnt) {
      quit = true;
      break;
    }
  }

  if (save_file)
    fclose(save_file);

  return NULL;
}



Camera::Camera() 
    : m_camera_id(0) {
    int ret = 0;
    RK_U32 u32Width = 1920;
    RK_U32 u32Height = 1080;

    const char *iq_file_dir = "/etc/iqfiles";
    const char *pDeviceName = "rkispp_scale0";

    RK_MPI_SYS_Init();
#ifdef RKAIQ
    rk_aiq_working_mode_t hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;
    RK_BOOL fec_enable = RK_FALSE;
    int fps = 30;

    SAMPLE_COMM_ISP_Init(m_camera_id, hdr_mode, fec_enable, iq_file_dir);
    SAMPLE_COMM_ISP_Run(m_camera_id);
    SAMPLE_COMM_ISP_SetFrameRate(m_camera_id, fps);
    //SAMPLE_COMM_ISP_SET_mirror(m_camera_id, 1);
#endif

    VI_CHN_ATTR_S vi_chn_attr;
    vi_chn_attr.pcVideoNode = pDeviceName;
    vi_chn_attr.u32BufCnt = 3;
    vi_chn_attr.u32Width = u32Width;
    vi_chn_attr.u32Height = u32Height;
    vi_chn_attr.enPixFmt = IMAGE_TYPE_NV12;
    vi_chn_attr.enWorkMode = VI_WORK_MODE_NORMAL;
    vi_chn_attr.enBufType = VI_CHN_BUF_TYPE_MMAP;
    ret = RK_MPI_VI_SetChnAttr(m_camera_id, m_vi_channel, &vi_chn_attr);
    ret |= RK_MPI_VI_EnableChn(m_camera_id, m_vi_channel);
    if (ret) {
        printf("Create VI[0] failed! ret=%d\n", ret);
        exit(-1);
    }

    pthread_t read_thread;
    pthread_create(&read_thread, NULL, GetMediaBuffer, NULL);
    ret = RK_MPI_VI_StartStream(m_camera_id, m_vi_channel);
    if (ret) {
        printf("Start VI[0] failed! ret=%d\n", ret);
        exit(-1);
    }

    while (!quit) {
        usleep(1000);
    }
}

Camera::~Camera() {
    RK_MPI_VI_DisableChn(m_camera_id, m_vi_channel);
    SAMPLE_COMM_ISP_Stop(m_camera_id);
}