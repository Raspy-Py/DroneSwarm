#include "rknn_utils.h"
#include "rknn_api.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>

#include <string>
#include <unordered_map>

std::string get_rknn_err_str(int errorCode) {
    static const std::unordered_map<int, std::string> errorMap = {
        {0, "RKNN_SUCC: execute succeed."},
        {-1, "RKNN_ERR_FAIL: execute failed."},
        {-2, "RKNN_ERR_TIMEOUT: execute timeout."},
        {-3, "RKNN_ERR_DEVICE_UNAVAILABLE: device is unavailable."},
        {-4, "RKNN_ERR_MALLOC_FAIL: memory malloc fail."},
        {-5, "RKNN_ERR_PARAM_INVALID: parameter is invalid."},
        {-6, "RKNN_ERR_MODEL_INVALID: model is invalid."},
        {-7, "RKNN_ERR_CTX_INVALID: context is invalid."},
        {-8, "RKNN_ERR_INPUT_INVALID: input is invalid."},
        {-9, "RKNN_ERR_OUTPUT_INVALID: output is invalid."},
        {-10, "RKNN_ERR_DEVICE_UNMATCH: the device is unmatch, please update rknn sdk and npu driver/firmware."},
        {-11, "RKNN_ERR_INCOMPATILE_PRE_COMPILE_MODEL: This RKNN model use pre_compile mode, but not compatible with current driver."},
        {-12, "RKNN_ERR_INCOMPATILE_OPTIMIZATION_LEVEL_VERSION: This RKNN model set optimization level, but not compatible with current driver."},
        {-13, "RKNN_ERR_TARGET_PLATFORM_UNMATCH: This RKNN model set target platform, but not compatible with current platform."},
        {-14, "RKNN_ERR_NON_PRE_COMPILED_MODEL_ON_MINI_DRIVER: This RKNN model is not a pre-compiled model, but the npu driver is mini driver."}
    };

    auto it = errorMap.find(errorCode);
    if (it != errorMap.end()) {
        return it->second;
    }
    return "Unknown error code.";
}

inline static int32_t __clip(float val, float min, float max)
{
    float f = val <= min ? min : (val >= max ? max : val);
    return f;
}

void f32_to_f16(uint16_t *f16, float *f32, int num)
{
    float *src = f32;
    uint16_t *dst = f16;
    int i = 0;

    for (; i < num; i++)
    {
        float in = *src;

        uint32_t fp32 = *((uint32_t *)&in);
        uint32_t t1 = (fp32 & 0x80000000u) >> 16; /* sign bit. */
        uint32_t t2 = (fp32 & 0x7F800000u) >> 13; /* Exponent bits */
        uint32_t t3 = (fp32 & 0x007FE000u) >> 13; /* Mantissa bits, no rounding */
        uint32_t fp16 = 0u;

        if (t2 >= 0x023c00u)
        {
            fp16 = t1 | 0x7BFF; /* Don't round to infinity. */
        }
        else if (t2 <= 0x01c000u)
        {
            fp16 = t1;
        }
        else
        {
            t2 -= 0x01c000u;
            fp16 = t1 | t2 | t3;
        }

        *dst = (uint16_t)fp16;

        src++;
        dst++;
    }
}

void qnt_f32_to_dfp(uint8_t *qnt, uint8_t type, int8_t fl, float *f32,
                    int num)
{
    float *src_ptr = f32;
    int i = 0;
    float dst_val = 0.0;

    switch (type)
    {
    case RKNN_TENSOR_INT8:
        for (; i < num; i++)
        {
            dst_val = (fl > 0) ? ((*src_ptr) * ((float)(1 << fl)))
                               : ((*src_ptr) / (float)(1 << -fl));
            *((int8_t *)qnt) = (int8_t)__clip(dst_val, -128, 127);
            src_ptr++;
            qnt++;
        }
        break;
    case RKNN_TENSOR_UINT8:
        for (; i < num; i++)
        {
            dst_val = (fl > 0) ? ((*src_ptr) * ((float)(1 << fl)))
                               : ((*src_ptr) / (float)(1 << -fl));
            *qnt = (uint8_t)__clip(dst_val, 0, 255);
            src_ptr++;
            qnt++;
        }
        break;
    case RKNN_TENSOR_INT16:
        for (; i < num; i++)
        {
            dst_val = (fl > 0) ? ((*src_ptr) * ((float)(1 << fl)))
                               : ((*src_ptr) / (float)(1 << -fl));
            *((int16_t *)qnt) = (int16_t)__clip(dst_val, -32768, 32767);
            src_ptr++;
            qnt += 2;
        }
        break;
    default:
        break;
    }
}

void qnt_f32_to_affine(uint8_t *qnt, uint8_t type, uint8_t zp, float scale,
                       float *f32, int num)
{
    float *src_ptr = f32;
    int i = 0;
    float dst_val = 0.0;

    switch (type)
    {
    case RKNN_TENSOR_INT8:
        for (; i < num; i++)
        {
            dst_val = ((*src_ptr) / scale) + zp;
            *((int8_t *)qnt) = (int8_t)__clip(dst_val, -128, 127);
            src_ptr++;
            qnt++;
        }
        break;
    case RKNN_TENSOR_UINT8:
        for (; i < num; i++)
        {
            dst_val = ((*src_ptr) / scale) + zp;
            *qnt = (uint8_t)__clip(dst_val, 0, 255);
            src_ptr++;
            qnt++;
        }
        break;
    case RKNN_TENSOR_INT16:
        for (; i < num; i++)
        {
            dst_val = ((*src_ptr) / scale) + zp;
            *((int16_t *)qnt) = (int16_t)__clip(dst_val, -32768, 32767);
            src_ptr++;
            qnt += 2;
        }
        break;
    default:
        break;
    }
}

void qnt_f32_to_none(uint8_t *qnt, uint8_t type, float *f32, int num)
{
    float *src_ptr = f32;
    int i = 0;

    switch (type)
    {
    case RKNN_TENSOR_INT8:
        for (; i < num; i++)
        {
            *((int8_t *)qnt) = (int8_t)__clip(*src_ptr, -128, 127);
            src_ptr++;
            qnt++;
        }
        break;
    case RKNN_TENSOR_UINT8:
        for (; i < num; i++)
        {
            *qnt = (uint8_t)__clip(*src_ptr, 0, 255);
            src_ptr++;
            qnt++;
        }
        break;
    case RKNN_TENSOR_INT16:
        for (; i < num; i++)
        {
            *((int16_t *)qnt) = (int16_t)__clip(*src_ptr, -32768, 32767);
            src_ptr++;
            qnt += 2;
        }
        break;
    default:
        break;
    }
}

unsigned char *load_model(const char *filename, int *model_size)
{
    FILE *fp = fopen(filename, "rb");
    if (fp == nullptr)
    {
        printf("fopen %s fail!\n", filename);
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    int model_len = ftell(fp);
    unsigned char *model = (unsigned char *)malloc(model_len);
    fseek(fp, 0, SEEK_SET);
    if (model_len != fread(model, 1, model_len, fp))
    {
        printf("fread %s fail!\n", filename);
        free(model);
        return NULL;
    }
    *model_size = model_len;
    if (fp)
    {
        fclose(fp);
    }
    return model;
}

unsigned char *load_image(const char *image_path, rknn_tensor_attr *input_attr)
{
    int req_height = 0;

    int req_width = 0;
    int req_channel = 0;

    switch (input_attr->fmt)
    {
    case RKNN_TENSOR_NHWC:
        req_height = input_attr->dims[2];
        req_width = input_attr->dims[1];
        req_channel = input_attr->dims[0];
        break;
    case RKNN_TENSOR_NCHW:
        req_height = input_attr->dims[1];
        req_width = input_attr->dims[0];
        req_channel = input_attr->dims[2];
        break;
    default:
        printf("meet unsupported layout\n");
        return NULL;
    }

    printf("w=%d,h=%d,c=%d, fmt=%d\n", req_width, req_height, req_channel, input_attr->fmt);

    int height = 0;
    int width = 0;
    int channel = 0;

    unsigned char *image_data = stbi_load(image_path, &width, &height, &channel, req_channel);
    if (image_data == NULL)
    {
        printf("load image failed!\n");
        return NULL;
    }

    if (width != req_width || height != req_height)
    {
        unsigned char *image_resized = (unsigned char *)STBI_MALLOC(req_width * req_height * req_channel);
        if (!image_resized)
        {
            printf("malloc image failed!\n");
            STBI_FREE(image_data);
            return NULL;
        }
        if (stbir_resize_uint8(image_data, width, height, 0, image_resized, req_width, req_height, 0, channel) != 1)
        {
            printf("resize image failed!\n");
            STBI_FREE(image_data);
            return NULL;
        }
        STBI_FREE(image_data);
        image_data = image_resized;
    }

    return image_data;
}


int load_image_to_buffer(const char *image_path, rknn_tensor_attr *input_attr, uint8_t *output_buffer)
{
    if (!output_buffer || !input_attr || !image_path) {
        printf("Invalid input parameters!\n");
        return -1;
    }

    int req_height = 0;
    int req_width = 0;
    int req_channel = 0;

    // Parse required dimensions based on format
    switch (input_attr->fmt)
    {
    case RKNN_TENSOR_NHWC:
        req_height = input_attr->dims[2];
        req_width = input_attr->dims[1];
        req_channel = input_attr->dims[0];
        break;
    case RKNN_TENSOR_NCHW:
        req_height = input_attr->dims[1];
        req_width = input_attr->dims[0];
        req_channel = input_attr->dims[2];
        break;
    default:
        printf("meet unsupported layout\n");
        return -1;
    }

    printf("w=%d,h=%d,c=%d, fmt=%d\n", req_width, req_height, req_channel, input_attr->fmt);

    // Load original image
    int width = 0;
    int height = 0;
    int channel = 0;
    
    unsigned char *temp_image = stbi_load(image_path, &width, &height, &channel, req_channel);
    if (!temp_image)
    {
        printf("load image [%s] failed!\n", image_path);
        return -1;
    }

    if (width != req_width || height != req_height)
    {
        // Need to resize - create temporary buffer
        unsigned char *temp_resized = (unsigned char *)malloc(req_width * req_height * req_channel);
        if (!temp_resized)
        {
            printf("malloc for resize failed!\n");
            STBI_FREE(temp_image);
            return -1;
        }

        // Perform resize
        if (stbir_resize_uint8(temp_image, width, height, 0, 
                              temp_resized, req_width, req_height, 0, 
                              channel) != 1)
        {
            printf("resize image failed!\n");
            STBI_FREE(temp_image);
            free(temp_resized);
            return -1;
        }

        // Copy resized data to output buffer
        memcpy(output_buffer, temp_resized, req_width * req_height * req_channel);
        
        // Clean up
        STBI_FREE(temp_image);
        free(temp_resized);
    }
    else
    {
        // No resize needed - copy directly to output buffer
        memcpy(output_buffer, temp_image, req_width * req_height * req_channel);
        STBI_FREE(temp_image);
    }

    return 0;
}

int process_input(unsigned char *src_buf, void **dst_buf, rknn_tensor_attr *in_attr,
    std::vector<float> mean, std::vector<float> scale,
    bool isReorder210, bool isNCHW)
{
    // check
    if (3 != mean.size() || src_buf == NULL)
    {
        return -1;
    }
    int img_height, img_width, img_channels = 0;
    if (isNCHW)
    {
        img_width = in_attr->dims[0];
        img_height = in_attr->dims[1];
        img_channels = in_attr->dims[2];
    }
    else
    {
        img_channels = in_attr->dims[0];
        img_width = in_attr->dims[1];
        img_height = in_attr->dims[2];
    }
    int HW = img_height * img_width;
    int ele_count = HW * img_channels;
    int ele_bytes = get_element_byte(in_attr);
    printf("total element count = %d, bytes per element = %d\n", ele_count,
           ele_bytes);
    float *pixel_f = (float *)malloc(ele_count * sizeof(float));

    // RGB2BGR
    if (isReorder210 == true)
    {
        printf("perform RGB2BGR\n");
        float *f_ptr = pixel_f;
        unsigned char *u_ptr = src_buf;
        for (int i = 0; i < HW; ++i)
        {
            f_ptr[2] = u_ptr[0];
            f_ptr[1] = u_ptr[1];
            f_ptr[0] = u_ptr[2];
            u_ptr += img_channels;
            f_ptr += img_channels;
        }
    }
    else
    {
        float *f_ptr = pixel_f;
        unsigned char *u_ptr = src_buf;
        for (int i = 0; i < ele_count; ++i)
        {
            f_ptr[i] = u_ptr[i];
        }
    }

    // normalize
    printf("perform normalize\n");
    float *f_ptr = pixel_f;
    for (int i = 0; i < HW; ++i)
    {
        for (int j = 0; j < img_channels; ++j)
        {
            f_ptr[j] -= mean[j];
            f_ptr[j] /= scale[j];
        }
        f_ptr += img_channels;
    }

    // quantize
    printf("perform quantize\n");
    uint8_t *qnt_buf = (uint8_t *)malloc(ele_count * ele_bytes);
    switch (in_attr->type)
    {
    case RKNN_TENSOR_FLOAT32:
        memcpy(qnt_buf, pixel_f, ele_count * ele_bytes);
        break;
    case RKNN_TENSOR_FLOAT16:
        f32_to_f16((uint16_t *)(qnt_buf), pixel_f, ele_count);
        break;
    case RKNN_TENSOR_INT16:
    case RKNN_TENSOR_INT8:
    case RKNN_TENSOR_UINT8:
        switch (in_attr->qnt_type)
        {
        case RKNN_TENSOR_QNT_DFP:
            qnt_f32_to_dfp(qnt_buf, in_attr->type, in_attr->fl, pixel_f, ele_count);
            break;
        case RKNN_TENSOR_QNT_AFFINE_ASYMMETRIC:
            qnt_f32_to_affine(qnt_buf, in_attr->type, in_attr->zp, in_attr->scale, pixel_f, ele_count);
            break;
        case RKNN_TENSOR_QNT_NONE:
            qnt_f32_to_none(qnt_buf, in_attr->type, pixel_f, ele_count);
        default:
            break;
        }
        break;
    default:
        break;
    }

    // NHWC ==> NCHW
    if (isNCHW)
    {
        printf("perform NHWC to NCHW\n");
        uint8_t *nchw_buf = (uint8_t *)malloc(ele_count * ele_bytes);
        uint8_t *dst_ptr = nchw_buf;
        uint8_t *src_ptr = qnt_buf;
        for (int i = 0; i < img_channels; ++i)
        {
            src_ptr = qnt_buf + i * ele_bytes;
            dst_ptr = nchw_buf + i * HW * ele_bytes;
            for (int j = 0; j < HW; ++j)
            {
                // dst_ptr[i*HW+j] = src_ptr[j*C+i];
                memcpy(dst_ptr, src_ptr, ele_bytes);
                src_ptr += img_channels * ele_bytes;
                dst_ptr += ele_bytes;
            }
        }
        *dst_buf = nchw_buf;
        free(qnt_buf);
    }
    else
    {
        *dst_buf = qnt_buf;
    }

    free(pixel_f);

    return 0;
}

int get_element_byte(rknn_tensor_attr *in_attr)
{
    int byte = 0;
    switch (in_attr->type)
    {
    case RKNN_TENSOR_FLOAT32:
        byte = 4;
        break;
    case RKNN_TENSOR_FLOAT16:
    case RKNN_TENSOR_INT16:
        byte = 2;
        break;
    case RKNN_TENSOR_INT8:
    case RKNN_TENSOR_UINT8:
        byte = 1;
        break;
    default:
        break;
    }
    return byte;
}

void print_tensor(rknn_tensor_attr *attr)
{
    printf("index=%d name=%s n_dims=%d dims=[%d %d %d %d] n_elems=%d size=%d "
           "fmt=%d type=%d qnt_type=%d fl=%d zp=%d scale=%f\n",
           attr->index, attr->name, attr->n_dims, attr->dims[3], attr->dims[2],
           attr->dims[1], attr->dims[0], attr->n_elems, attr->size, 0, attr->type,
           attr->qnt_type, attr->fl, attr->zp, attr->scale);
}

std::chrono::high_resolution_clock::time_point now() {
    return std::chrono::high_resolution_clock::now();
}
