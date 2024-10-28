#include "sample_common.h"
#include <easymedia/rkmedia_api.h>
#include <easymedia/rkmedia_vdec.h>


// typedef struct rkMPP_CHN_S {
//   MOD_ID_E enModId;
//   RK_S32 s32DevId;
//   RK_S32 s32ChnId;
// } MPP_CHN_S;


// typedef enum rkMOD_ID_E {
//   RK_ID_UNKNOW = 0,
//   RK_ID_VB,
//   RK_ID_SYS,
//   RK_ID_VDEC,
//   RK_ID_VENC,
//   RK_ID_VO,
//   RK_ID_VI,
//   RK_ID_RGA,
//   RK_ID_VMIX,
//   RK_ID_MUXER,
//   RK_ID_BUTT,
// } MOD_ID_E;

struct RKPipeline {
    MPP_CHN_S src_channel;
    MPP_CHN_S dst_channel;

    int Bind(){
        return RK_MPI_SYS_Bind(&src_channel, &dst_channel);
    }
    int UnBind() {
        return RK_MPI_SYS_UnBind(&src_channel, &dst_channel);
    }
};


class RKModule {
public:
    RKModule(MOD_ID_E mode_id = RK_ID_UNKNOW, RK_S32 channel_id = 0, RK_S32 device_id = 0)
        : m_mode_id(mode_id), m_device_id(device_id), m_channel_id(channel_id) {}

protected:
    MOD_ID_E m_mode_id;
    RK_S32 m_device_id;
    RK_S32 m_channel_id;
};

