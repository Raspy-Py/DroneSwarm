#ifndef ML_TOOLS_COMMON_RKMEDIA_UTILS_H
#define ML_TOOLS_COMMON_RKMEDIA_UTILS_H

int create_vi_channel(int camera_id, int channel_id, int width, int height, int buf_cnt, int fps, int pix_fmt, int work_mode, int buf_type);

int create_rga_buffer(int width, int height, int model_input_size, unsigned char *rga_buffer, unsigned char *rga_buffer_model_input);

#endif // ML_TOOLS_COMMON_RKMEDIA_UTILS_H