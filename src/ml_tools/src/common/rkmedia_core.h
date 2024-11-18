#ifndef ML_TOOLS_COMMON_RKMEDIA_CORE_H
#define ML_TOOLS_COMMON_RKMEDIA_CORE_H

#if 0

// std
#include <getopt.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <map>
#include <memory>
#include <vector>
#include <iostream>

// rkmedia
#include <rga/im2d.h>
#include <rga/rga.h>
#include <rkmedia_api.h>
#include <rkmedia_venc.h>
#include <rknn_api.h>

// internal
#ifdef RKAIQ
#include "common/sample_common.h"
#endif
#include "librtsp/rtsp_demo.h"
#include "rknn_utils.h"

namespace rk {

/*
* Base Classes
*/

class MediaNode {
public:
    MediaNode() = delete;
    MediaNode(MOD_ID_E mod_id, RK_S32 chn_id, RK_S32 dev_id = -1) 
        : m_mod_id(mod_id), m_chn_id(chn_id), m_dev_id(dev_id) {}

    MOD_ID_E get_mod_id() const { return m_mod_id; }
    RK_S32 get_chn_id() const { return m_chn_id; }
    RK_S32 get_dev_id() const { return m_dev_id; }

protected:
    MOD_ID_E m_mod_id;
    RK_S32 m_chn_id;

    // optional
    RK_S32 m_dev_id;
};


class Pipeline {};

class PipelineBuilder {
public:
    PipelineBuilder& add_node(const std::string& parent_name, const std::string& name, std::unqiue_ptr<MediaNode>&& node){
        m_nodes[node_name] = {parent_name, std::move(node)};
        return *this;
    }

    std::unique_ptr<Pipeline> build() {
        // TODO: pipeline build logic   

        return std::make_unique<Pipeline>(m_nodes);
    }
private:
    struct NodeInfo {
        std::string parent_name;
        std::unique_ptr<MediaNode> node;
    };
    std::map<std::string, NodeInfo> m_nodes;
};


// Camera
// Video encoder
// RGA
// NPU
// CPU
// RTSP


/*
* Global Functions
*/

// This one should be called before any stuff involving several channels
void init() {
    RK_MPI_SYS_Init();
}


void bind(MediaChannel &src, MediaChannel &dst) {
    MPP_CHN_S src_channel{};
    src_channel.enModId = src.get_mod_id();
    src_channel.s32ChnId = src.get_chn_id();
    MPP_CHN_S dst_channel{};
    dst_channel.enModId = dst.get_mod_id();
    dst_channel.s32ChnId = dst.get_chn_id();
    RKNN_FATAL(RK_MPI_SYS_Bind(&src_channel, &dst_channel));
}

void unbind(MediaChannel &src, MediaChannel &dst) {
    MPP_CHN_S src_channel{};
    src_channel.enModId = src.get_mod_id();
    src_channel.s32ChnId = src.get_chn_id();
    MPP_CHN_S dst_channel{};
    dst_channel.enModId = dst.get_mod_id();
    dst_channel.s32ChnId = dst.get_chn_id();
    RKNN_FATAL(RK_MPI_SYS_UnBind(&src_channel, &dst_channel));
}

void set_output_callback(MediaChannel &channel, void (*callback)(MEDIA_BUFFER)) {
    MPP_CHN_S chn;
    chn.enModId = channel.get_mod_id();
    chn.s32ChnId = channel.get_chn_id();
    RKNN_FATAL(RK_MPI_SYS_RegisterOutCb(&chn, callback));
}


/*
* @brief Video Input 
*/
namespace vi {
    class Channel : public MediaNode {
    public:
        Channel() = delete;
        Channel(RK_S32 chn_id, RK_S32 dev_id) : MediaChannel(chn_id, dev_id) {

        }
    };

    class Node {

    };

    std::unique_ptr<Node> create_node() {
        return std::make_unique<Node>();
    }
};

/*
* @brief Video Encode
*/
namespace venc {};

/*
* @brief Raster Graphic Accelerator
*/
namespace rga {};

/*
* @brief Video Transmission
*/
namespace rtsp {};


};

#endif // 0


#endif // ML_TOOLS_COMMON_RKMEDIA_CORE_H