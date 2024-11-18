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
#include "rknn_utils.h"


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


class Model {
public:
    Model(const std::string& model_path) {
        m_model_path = model_path;

        printf("Loading model...\n");
        m_model = load_model(m_model_path.c_str(), &m_model_len);
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
            print_tensor(m_input_attrs.data() + i);
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
            print_tensor(m_output_attrs.data() + i);
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

    std::vector<rknn_tensor_attr>& get_input_info() { return m_input_attrs; }
    std::vector<rknn_tensor_attr>& get_output_info() { return m_output_attrs; }
    std::vector<std::vector<uint8_t>>& get_output_buffers() { return m_output_buffers; }

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
};

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Usage: %s model_path img_path\n", argv[0]);
        return -1;
    }

    const char *model_path = argv[1];
    const char *img_path = argv[2];

    Model model(model_path);

    auto& inputs_info = model.get_input_info();
    std::vector<std::vector<uint8_t>> input_data(inputs_info.size());
    input_data[0].resize(inputs_info[0].size);
    if (load_image_to_buffer(img_path, &inputs_info[0], input_data[0].data())) {
        printf("Failed to load image\n");
        exit(-1);
    }

    model.run(input_data);

    int runs = 100;
    auto start = now();
    for (int i = 0; i < runs; i++) {
        model.run(input_data);
    }
    auto end = now();
    printf("Avarage inference time: %f ms\n", float(to_us(end - start)) / 1000.f / runs);
    exit(0);
}
