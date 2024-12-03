// std
#include <getopt.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <vector>
#include <iostream>
#include <thread>
#include <atomic>
#include <cstring>

// rkmedia
#include "sample_common.h"
#include <easymedia/rkmedia_api.h>
#include <easymedia/rkmedia_vdec.h>
#include <easymedia/rkmedia_rga.h>

#include <rknn_api.h>
#include <rtsp_demo.h>
#include "rknn_utils.h"

#include <Eigen/Dense>
#include "thread_safe_queue.h"

#include "alike/dkd.h"

/*
0. Load RKNN model
 # in a loop:
1. Get image from camera
2. Convert image from NV12 to RGB [DONE in RGA]
3. Quantize the image
4. Transpose the image from HWC to CHW [CAN BE PERFORMED BY DRIVER]
5. Run the model
6. Get the output
*/


const char* DEVICE_NAME = "rkispp_scale0";
const char* IQ_FILE_DIR = "/etc/iqfiles";
const char* STREAM_ADDRESS = "/live/main_stream";

char OUTPUT_FILE[] = "/tmp/output.rgb";

RK_U32 STREAM_PORT = 554;
RK_S32 VI_CHANNEL = 0;
RK_S32 CAMERA_ID = 0;
RK_S32 RGA_CHANNEL = 0;
RK_S32 RGA_DEVICE_ID = 0;

RK_U32 IMAGE_WIDTH = 1920;
RK_U32 IMAGE_HEIGHT = 1080;


typedef struct {
  char *filePath;
  int frameCount;
} OutputArgs;

static std::atomic_bool quit{false};
static void sigtermHandler(int sig) {
  quit = true;
}

class VideoInput {
public:
    VideoInput(RK_S32 device_id, RK_S32 vi_channel, RK_U32 width, RK_U32 height)
        : m_device_id(device_id), m_vi_channel(vi_channel), m_width(width), m_height(height) {
        
        RK_MPI_SYS_Init();

        // Start the camera
        SAMPLE_COMM_ISP_Init(m_device_id, m_hdr_mode, m_fec_enable, m_iq_files_dir);
        SAMPLE_COMM_ISP_Run(m_device_id);
        SAMPLE_COMM_ISP_SetFrameRate(m_device_id, m_input_frame_rate);


        // Create a video input channel
        VI_CHN_ATTR_S vi_chn_attr;
        vi_chn_attr.pcVideoNode = m_device_name;
        vi_chn_attr.u32BufCnt = m_input_buf_cnt;
        vi_chn_attr.u32Width = m_width;
        vi_chn_attr.u32Height = m_height;
        vi_chn_attr.enPixFmt = m_input_pix_fmt;
        vi_chn_attr.enWorkMode = VI_WORK_MODE_NORMAL;
        // Not sure about this one. Maybe DMA is better for zero copy mode
        vi_chn_attr.enBufType = VI_CHN_BUF_TYPE_MMAP;

        m_pipeline_id = create_pipeline();
        if (m_pipeline_id < 0) {
            printf("[VideoInput ERROR] No free video pipelines available!\n");
            exit(-1);
        }
        else {
            printf("[VideoInput INFO] Using pipeline %d\n", m_pipeline_id);
        }

        if(RK_MPI_VI_SetChnAttr(m_pipeline_id, m_vi_channel, &vi_chn_attr) != 0){
            printf("[VideoInput ERROR] Create VI[%d] failed!\n", m_vi_channel);
            release_pipeline();
            exit(-1);
        }

        if(RK_MPI_VI_EnableChn(m_pipeline_id, m_vi_channel)){
            printf("[VideoInput ERROR] Enable VI[%d] failed!\n", m_vi_channel);
            release_pipeline();
            exit(-1);
        }
    }
    
    ~VideoInput() {
        RK_MPI_VI_DisableChn(m_pipeline_id, m_vi_channel);
        SAMPLE_COMM_ISP_Stop(m_device_id);
        release_pipeline();
    } 

    RK_S32 get_device_id() const { return m_device_id; }
    RK_S32 get_vi_channel() const { return m_vi_channel; }
    RK_U32 get_width() const { return m_width; }
    RK_U32 get_height() const { return m_height; }
    IMAGE_TYPE_E get_format() const { return m_input_pix_fmt; }

private:
    VI_PIPE create_pipeline() {
        if (m_pipeline_id > 0){
            printf("[VideoInput WARNING] Pipeline already exists. Recreating...");
            release_pipeline();
        }

        for (size_t i = 0; i < s_occupied_pipelines.size(); i++) {
            if (!s_occupied_pipelines[i]) {
                s_occupied_pipelines[i] = true;
                return i;
            }
        }
        return -1;
    }

    void release_pipeline() {
        if (m_pipeline_id >= 0){
            s_occupied_pipelines[m_pipeline_id] = false;
            m_pipeline_id = -1;
        }   
    }

private:
    RK_S32 m_device_id;
    RK_S32 m_vi_channel;
    RK_U32 m_width;
    RK_U32 m_height;
    VI_PIPE m_pipeline_id = -1;

    /**
     * Camera configuration
     */
    
    // Seems to work both with 2 and 3. Probably it's like a swapchain,
    // more buffers - less tearing but higher latency
    RK_U32 m_input_buf_cnt = 3;

    // Frame rate
    RK_U32 m_input_frame_rate = 30;

    // It always seems to be NV12
    IMAGE_TYPE_E m_input_pix_fmt = IMAGE_TYPE_NV12;

    // No idea what this is
    RK_BOOL m_fec_enable = RK_FALSE;

    // No HDR
    rk_aiq_working_mode_t m_hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;

    // This thing controls zoom. "rkispp_scale0" means no zoom
    const char* m_device_name = "rkispp_scale0";
    
    // Folder with sensor calibration files. Without it default settings will be used
    // and they are no fit for our camera
    const char* m_iq_files_dir = "/etc/iqfiles";

    /**
     * Static stuff
     */

    // TO-FIX: this should be shared across the system 
    static std::array<bool, 40> s_occupied_pipelines;
};

std::array<bool, 40> VideoInput::s_occupied_pipelines{};


// Resizing and format convertion supported
class VideoTransform {
public:
    VideoTransform(RK_S32 device_id, RK_S32 rga_channel,
        const VideoInput& video_input,
        RK_U32 out_width, RK_U32 out_height, 
        IMAGE_TYPE_E out_format = IMAGE_TYPE_RGB888)
        : m_device_id(device_id), m_rga_channel(rga_channel)
    {
        RK_U32 in_width = video_input.get_width();
        RK_U32 in_height = video_input.get_height();
        IMAGE_TYPE_E in_format = video_input.get_format();

                                //img fmt,    x, y, width,    height,   hor. stride, vert. stride
        RGA_ATTR_S rgaAttr = {  {in_format,   0, 0, in_width, in_height, in_width, in_height},
                                {out_format, 0, 0, out_width, out_height, out_width, out_height},
                                //rotation, buffer pool, buffer pool count, flip
                                 0,         RK_TRUE,     2,                 RGA_FLIP_NULL };

        RK_MPI_RGA_CreateChn(m_rga_channel, &rgaAttr);

        m_src_channel = { RK_ID_VI, video_input.get_device_id(), video_input.get_vi_channel() };
        m_dest_channel = { RK_ID_RGA, m_device_id, m_rga_channel };
        RK_MPI_SYS_Bind(&m_src_channel, &m_dest_channel);
    }

    ~VideoTransform() {
        RK_MPI_SYS_UnBind(&m_src_channel, &m_dest_channel);
        RK_MPI_RGA_DestroyChn(m_rga_channel);
    }

private:
    RK_S32 m_device_id;
    RK_S32 m_rga_channel;

    // Bind info
    MPP_CHN_S m_src_channel;
    MPP_CHN_S m_dest_channel;
};


class Model {
public:
    Model(const std::string& model_path) {
        m_model_path = model_path;

        printf("Loading model...\n");
        m_model = utils::load_model(m_model_path.c_str(), &m_model_len);
        RKNN_FATAL(rknn_init(&m_ctx, m_model, m_model_len, 0));
        printf("Successfully loaded the model.\n");

        // Get Model Input Output Info
        rknn_input_output_num io_num;
        RKNN_FATAL(rknn_query(m_ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num)));
        printf("Model input num: %d, output num: %d\n", io_num.n_input, io_num.n_output);

        printf("Input tensors:\n");
        m_input_attrs.resize(io_num.n_input);
        m_inputs.resize(io_num.n_input);
        for (int i = 0; i < io_num.n_input; i++)
        {
            m_input_attrs[i].index = i;
            RKNN_FATAL(rknn_query(m_ctx, RKNN_QUERY_INPUT_ATTR, m_input_attrs.data() + i, sizeof(rknn_tensor_attr)));
            utils::print_tensor(m_input_attrs.data() + i);
        }

        printf("Output tensors:\n");
        m_outputs.resize(io_num.n_output);
        m_output_attrs.resize(io_num.n_output);
        m_output_buffers.resize(io_num.n_output);
        for (int i = 0; i < io_num.n_output; i++)
        {
            m_output_attrs[i].index = i;
            RKNN_FATAL(rknn_query(m_ctx, RKNN_QUERY_OUTPUT_ATTR, m_output_attrs.data() + i, sizeof(rknn_tensor_attr)));
            m_output_buffers[i].resize(m_output_attrs[i].size);
            utils::print_tensor(m_output_attrs.data() + i);
        }
    }

    ~Model() {
        if (m_ctx >= 0) {
            rknn_destroy(m_ctx);
        }
        if (m_model) {
            free(m_model);
        }
    }

    void run(std::vector<std::vector<uint8_t>>& input_data) {
        for (int i = 0; i < m_inputs.size(); i++) {
            // Set Input Data
            m_inputs[i].index = i;
            m_inputs[i].size = m_input_attrs[i].size;
            m_inputs[i].buf = reinterpret_cast<void*>(input_data[i].data());

            // Delegate input tensor transposing to driver
            m_inputs[i].pass_through = 0;
            m_inputs[i].fmt = RKNN_TENSOR_NHWC; // input is in HxWxC format
            m_inputs[i].type = RKNN_TENSOR_UINT8;
        }
        RKNN_CHECK(rknn_inputs_set(m_ctx, m_inputs.size(), m_inputs.data()));

        // Run
        RKNN_CHECK(rknn_run(m_ctx, nullptr));

        // Get Output
        for (size_t i = 0; i < m_outputs.size(); i++) {
            m_outputs[i].index = i;
            m_outputs[i].want_float = 0; // do not dequantize the output
            m_outputs[i].is_prealloc = 1; // use preallocated buffer
            m_outputs[i].buf = m_output_buffers[i].data();
            m_outputs[i].size = m_output_buffers[i].size() * sizeof(m_output_buffers[i][0]);
        }

        RKNN_CHECK(rknn_outputs_get(m_ctx, m_outputs.size(), m_outputs.data(), nullptr));
    }

    // Zero-copy version of run()
    void run_mapped(std::vector<std::vector<uint8_t>>& input_data) {
        // Ensure CPU cache coherence for input data
        //RKNN_CHECK(rknn_inputs_sync(m_ctx, m_input_mems.size(), m_input_mems.data()));

        for (int i = 0; i < m_inputs.size(); i++) {
            // Set Input Data
            m_inputs[i].index = i;
            m_inputs[i].size = m_input_attrs[i].size;
            m_inputs[i].buf = reinterpret_cast<void*>(input_data[i].data());

            // Delegate input tensor transposing to driver
            m_inputs[i].pass_through = 0;
            m_inputs[i].fmt = RKNN_TENSOR_NHWC; // input is in HxWxC format
            m_inputs[i].type = RKNN_TENSOR_UINT8;
        }
        RKNN_CHECK(rknn_inputs_set(m_ctx, m_inputs.size(), m_inputs.data()));

        RKNN_CHECK(rknn_run(m_ctx, nullptr));

        // Ensure CPU cache coherence for output data
        RKNN_CHECK(rknn_outputs_sync(m_ctx, m_output_mems.size(), m_output_mems.data()));
    }

    void run_mapped(uint8_t* data) {
        // Set Input Data
        m_inputs[0].index = 0;
        m_inputs[0].size = m_input_attrs[0].size;
        m_inputs[0].buf = reinterpret_cast<void*>(data);

        // Delegate input tensor transposing to driver
        m_inputs[0].pass_through = 0;
        m_inputs[0].fmt = RKNN_TENSOR_NHWC; // input is in HxWxC format
        m_inputs[0].type = RKNN_TENSOR_UINT8;
        
        RKNN_CHECK(rknn_inputs_set(m_ctx, m_inputs.size(), m_inputs.data()));

        RKNN_CHECK(rknn_run(m_ctx, nullptr));

        // Ensure CPU cache coherence for output data
        RKNN_CHECK(rknn_outputs_sync(m_ctx, m_output_mems.size(), m_output_mems.data()));
    }

    void map_io(){
        //m_input_mems.resize(m_inputs.size());
        //RKNN_FATAL(rknn_inputs_map(m_ctx, m_input_mems.size(), m_input_mems.data()));

        m_output_mems.resize(m_outputs.size());
        printf("[Model INFO] m_outputs=%d\n", m_outputs.size());
        RKNN_FATAL(rknn_outputs_map(m_ctx, m_output_mems.size(), m_output_mems.data()));
    }

    void unmap_io(){
        //RKNN_FATAL(rknn_inputs_map(m_ctx, m_inputs.size(), m_input_mems.data()));
        //m_input_mems.clear();

        RKNN_FATAL(rknn_outputs_map(m_ctx, m_output_mems.size(), m_output_mems.data()));
        m_output_mems.clear();
    }


    std::vector<rknn_tensor_attr>& get_input_info() { return m_input_attrs; }
    std::vector<rknn_tensor_attr>& get_output_info() { return m_output_attrs; }
    std::vector<std::vector<uint8_t>>& get_output_buffers() { return m_output_buffers; }

    // Only call when using zero-copy mode
    std::vector<rknn_tensor_mem>& get_output_mems() { return m_output_mems; }
    std::vector<rknn_tensor_mem>& get_input_mems() { return m_input_mems; }

private:
    int m_model_len;
    uint8_t *m_model;
    std::string m_model_path;
    rknn_context m_ctx;

    std::vector<rknn_tensor_attr> m_input_attrs;
    std::vector<rknn_tensor_attr> m_output_attrs;

    std::vector<rknn_input> m_inputs;
    std::vector<rknn_output> m_outputs;

    std::vector<std::vector<uint8_t>> m_output_buffers;

    // For mapped version
    std::vector<rknn_tensor_mem> m_input_mems;
    std::vector<rknn_tensor_mem> m_output_mems;
};

void extract_scoremap(uint8_t* input, uint8_t* scoremap, int height, int width) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Extract 97th channel (index 96)
            scoremap[y * width + x] = input[(96 * height * width) + (y * width + x)];
        }
    }
}

void extract_scoremap_transposed(uint8_t* input, uint8_t* scoremap, int height, int width) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            scoremap[x * height + y] = input[(96 * height * width) + (y * width + x)];
        }
    }
}

void extract_scoremap_reordered(uint8_t* input, uint8_t* scoremap, int height, int width) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            scoremap[y * width + x] = input[(96 * height * width) + (x * height + y)];
        }
    }
}

// Isn't used on edge
void perform_matching(const Eigen::MatrixXf& desc1, const Eigen::MatrixXf& desc2, std::vector<std::pair<int, int>>& matches, float threshold) {
    Eigen::MatrixXf sim = desc1 * desc2.transpose();
    sim = (sim.array() >= threshold).select(sim, Eigen::MatrixXf::Zero(sim.rows(), sim.cols()));

    Eigen::VectorXi nn12 = Eigen::VectorXi::Zero(sim.rows());
    Eigen::VectorXi nn21 = Eigen::VectorXi::Zero(sim.cols());

    for (int i = 0; i < sim.rows(); ++i) {
        sim.row(i).maxCoeff(&nn12(i));
    }

    for (int i = 0; i < sim.cols(); ++i) {
        sim.col(i).maxCoeff(&nn21(i));
    }

    for (int i = 0; i < sim.rows(); ++i) {
        if (i == nn21[nn12[i]] && sim.row(i)[nn12[i]]){
        matches.emplace_back(i, nn12[i]);
        }
    }
}

struct Frame {
    uint32_t id;
    Eigen::MatrixXi keypoints;
    Eigen::MatrixXf descriptors;
};

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: %s model_path", argv[0]);
        return -1;
    }

    const char *model_path = argv[1];
    //const char *img_path = argv[2];

    Model model(model_path);
    model.map_io();

    auto& input_mems = model.get_input_mems();
    auto& input_info = model.get_input_info();
    uint8_t* input_model_addr = reinterpret_cast<uint8_t*>(input_mems[0].logical_addr);

    auto& output_mems = model.get_output_mems();
    auto& output_info = model.get_output_info();
    uint8_t* output_model_addr = reinterpret_cast<uint8_t*>(output_mems[0].logical_addr);

    VideoInput video_input(CAMERA_ID, VI_CHANNEL, IMAGE_WIDTH, IMAGE_HEIGHT);

    uint32_t model_width = input_info[0].dims[0];
    uint32_t model_height = input_info[0].dims[1];

    VideoTransform video_transform(RGA_DEVICE_ID, RGA_CHANNEL, video_input, model_width, model_height);

    auto& odms = output_info[0].dims;
    size_t D = odms[2] - 1, H = odms[1], W = odms[0];

    ThreadSafeQueue<std::vector<uint8_t>> feature_map_queue;

    auto feature_extractor_worker = [&]() {
        int frame_count = 0;
        uint32_t output_tensor_size = (D + 1) * H * W;
        MEDIA_BUFFER media_buffer = NULL;

        while (!quit) {
            media_buffer = RK_MPI_SYS_GetMediaBuffer(RK_ID_RGA, 0, -1);
            printf("[feature_extractor_worker INFO] Got media buffer\n");
            if (!media_buffer) break;

            uint8_t* input_data = reinterpret_cast<uint8_t*>(RK_MPI_MB_GetPtr(media_buffer));
            size_t input_size = RK_MPI_MB_GetSize(media_buffer);

            // {// Write image to file
            //     std::string img_file = "imgs/img" + std::to_string(frame_count++) + ".rgb";
            //     std::ofstream image_file(img_file, std::ios::binary | std::ios::out);
            //     if (!image_file) {
            //         std::cerr << "Failed to open output file for keypoints." << std::endl;
            //         continue;
            //     }
            //     image_file.write(
            //         reinterpret_cast<const char*>(input_data), 
            //         sizeof(input_data[0]) * input_size
            //     );
            //     image_file.close();
            // }

            model.run_mapped(input_data);

            // Get the output
            std::vector<uint8_t> feature_map(output_model_addr, output_model_addr + output_tensor_size);
            feature_map_queue.push(std::move(feature_map));

            RK_MPI_MB_ReleaseBuffer(media_buffer);
        }

        // Poison pill
        feature_map_queue.push({}); 
    };
    
    DKD dkd(200, 1, 4);
    ThreadSafeQueue<Frame> frame_queue;


    auto keypoint_detector_worker = [&](){
        int frame_count = 0;
        while (!quit) {
            std::vector<uint8_t> feature_map = feature_map_queue.wait_and_pop();
            if (feature_map.empty()) {
                // Took a poison pill
                break;
            } 

            // Splite the feature map into keypoints and descriptors
            //size_t D = 96, H = 160, W = 256;
            size_t descriptors_offset = H * W;
            size_t scoremap_offset = D * H * W;
            Eigen::MatrixXi keypoints;
            Eigen::MatrixXf descriptors;
            // I hope the memory layout matches the one I expect here.... please....
            dkd.run(feature_map.data() + scoremap_offset, feature_map.data(), keypoints, descriptors, D, H, W);
            
            // Submit frame for sending
            Frame frame;
            frame.id = frame_count++;
            frame.keypoints = keypoints;
            frame.descriptors = descriptors;
            frame_queue.push(std::move(frame));

            // {// Write keypoints to file
            //     std::string scrs_path = "scrs/scrs" + std::to_string(frame_count) + ".hm";
            //     std::ofstream scores_file(scrs_path, std::ios::binary | std::ios::out);
            //     if (!scores_file) {
            //         std::cerr << "Failed to open output file for keypoints." << std::endl;
            //         continue;
            //     }
            //     scores_file.write(
            //         reinterpret_cast<const char*>(feature_map.data() + scoremap_offset), 
            //         sizeof(feature_map[0]) * H * W
            //     );
            //     scores_file.close();
            // }

            // {// Write keypoints to file
            //     std::string kpts_path = "kpts/kpts" + std::to_string(frame_count++) + ".bin";
            //     std::ofstream keypoints_file(kpts_path, std::ios::binary | std::ios::out);
            //     if (!keypoints_file) {
            //         std::cerr << "Failed to open output file for keypoints." << std::endl;
            //         continue;
            //     }
            //     keypoints_file.write(
            //         reinterpret_cast<const char*>(keypoints.data()), 
            //         sizeof(keypoints(0, 0)) * keypoints.cols() * keypoints.rows()
            //     );
            //     keypoints_file.close();
            // }
        }
    };


    // Create asio broadcaster here and start sending frames.

    auto network_server_worker = [&](){

    };

    std::thread feature_extractor_thread(feature_extractor_worker);
    std::thread keypoint_detector_thread(keypoint_detector_worker);
    std::thread network_server_thread(network_server_worker);
    feature_extractor_thread.join();
    keypoint_detector_thread.join();
    network_server_thread.join();

    model.unmap_io();
    
    return 0;
}
