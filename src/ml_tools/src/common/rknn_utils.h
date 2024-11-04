// Numerous utility functions for working with Rockchip's RKNN API.
// Mostly copied from their examples, with some minor modifications.
// They insist on putting their copyright notice so have it, Rockchip:

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

#ifndef ML_TOOLS_COMMON_RKNN_UTILS_H
#define ML_TOOLS_COMMON_RKNN_UTILS_H

#include <fstream>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <vector>
#include <rknn_api.h>

#include <stb_image.h>
#include <stb_image_resize.h>


// For more convenient error handling

#define RKNN_CHECK_BASE(expression, file, line) \
    do {                                        \
        int ret = (expression);                 \
        if (ret < 0)                            \
        {                                       \
            printf("[RKNN ERROR] %s:%d: %s", file, line, #expression); \
            printf("[RKNN ERROR] Failed with error code = %d\n", ret); \
        }                                       \
    } while(0)
#define RKNN_CHECK(expression) RKNN_CHECK_BASE(expression, __FILE__, __LINE__)

#define RKNN_FATAL_BASE(expression, file, line) \
    do {                                        \
        int ret = (expression);                 \
        if (ret < 0)                            \
        {                                       \
            printf("[RKNN ERROR] %s:%d: %s", file, line, #expression); \
            printf("[RKNN ERROR] Failed with error code = %d\n", ret); \
            return ret;                         \
        }                                       \
    } while(0)
#define RKNN_FATAL(expression) RKNN_FATAL_BASE(expression, __FILE__, __LINE__)


// Quantization utils from Rockchip

void f32_to_f16(uint16_t *f16, float *f32, int num);

void qnt_f32_to_dfp(uint8_t *qnt, uint8_t type, int8_t fl, float *f32, int num);

void qnt_f32_to_affine(uint8_t *qnt, uint8_t type, uint8_t zp, float scale, float *f32, int num);

void qnt_f32_to_none(uint8_t *qnt, uint8_t type, float *f32, int num);


// Some other stuff from RKNN models inference examples

unsigned char *load_image(const char *image_path, rknn_tensor_attr *input_attr);

unsigned char *load_model(const char *filename, int *model_size);

int get_element_byte(rknn_tensor_attr *in_attr);

int process_input(unsigned char *src_buf, void **dst_buf, rknn_tensor_attr *in_attr, 
    std::vector<float> mean, std::vector<float> scale, 
    bool isReorder210, bool isNCHW);

void print_tensor(rknn_tensor_attr *attr);

#endif /*ML_TOOLS_COMMON_RKNN_UTILS_H */