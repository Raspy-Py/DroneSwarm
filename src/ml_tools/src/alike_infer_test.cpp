// Copyright (c) 2021 by Rockchip Electronics Co., Ltd. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <vector>
#include <iostream>

#include <rknn_api.h>
#include "rknn_utils.h"

int main(int argc, char **argv)
{
    const int MODEL_IN_WIDTH = 256;
    const int MODEL_IN_HEIGHT = 192;
    const int MODEL_IN_CHANNELS = 3;
    bool isNCHW = false;
    bool isReorder210 = false;
    std::vector<float> mean({103.94, 116.78, 123.68});
    std::vector<float> scale({58.82, 58.82, 58.82});

    rknn_context ctx;
    int ret;
    int model_len = 0;
    unsigned char *model;

    const char *model_path = argv[1];
    const char *img_path = argv[2];

    // Load RKNN Model
    model = utils::load_model(model_path, &model_len);
    RKNN_FATAL(rknn_init(&ctx, model, model_len, 0));

    // Get Model Input Output Info
    rknn_input_output_num io_num;
    RKNN_FATAL(rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num)));
    printf("model input num: %d, output num: %d\n", io_num.n_input,
           io_num.n_output);

    printf("input tensors:\n");
    std::vector<rknn_tensor_attr> input_attrs(io_num.n_input);
    for (int i = 0; i < io_num.n_input; i++)
    {
        input_attrs[i].index = i;
        RKNN_FATAL(rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, input_attrs.data() + i, sizeof(rknn_tensor_attr)));
        utils::print_tensor(input_attrs.data() + i);
    }

    printf("output tensors:\n");
    std::vector<rknn_tensor_attr> output_attrs(io_num.n_output);
    for (int i = 0; i < io_num.n_output; i++)
    {
        output_attrs[i].index = i;
        RKNN_FATAL(rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, output_attrs.data() + i, sizeof(rknn_tensor_attr)));
        utils::print_tensor(output_attrs.data() + i);
    }

    // Load image
    unsigned char *img_data = NULL;
    img_data = utils::load_image(img_path, input_attrs.data());
    if (!img_data)
    {
        printf("[ERROR] Load image failed!\n");
        return -1;
    }

    // rknn model need nchw layout input
    if (input_attrs[0].fmt == RKNN_TENSOR_NCHW)
    {
        isNCHW = true;
    }

    // process input when pass_through = 1
    void *in_data = NULL;
    utils::process_input(img_data, &in_data, input_attrs.data(), mean, scale, isReorder210, isNCHW);

    // Set Input Data
    rknn_input inputs[1]{};
    inputs[0].index = 0;
    inputs[0].type = RKNN_TENSOR_UINT8;
    inputs[0].size = input_attrs[0].size;
    inputs[0].fmt = RKNN_TENSOR_NCHW;
    inputs[0].buf = in_data;
    inputs[0].pass_through = 1;

    RKNN_CHECK(rknn_inputs_set(ctx, io_num.n_input, inputs));

    // Run
    printf("rknn_run\n");
    RKNN_CHECK(rknn_run(ctx, nullptr));

    // Get Output
    rknn_output outputs[1]{};
    outputs[0].want_float = 1;
    RKNN_CHECK(rknn_outputs_get(ctx, 1, outputs, NULL));

    // Release rknn_outputs
    rknn_outputs_release(ctx, 1, outputs);

    // Release
    if (ctx >= 0)
    {
        rknn_destroy(ctx);
    }
    if (model)
    {
        free(model);
    }
    stbi_image_free(img_data);
    return 0;
}
