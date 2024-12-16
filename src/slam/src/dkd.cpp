#include "dkd.h"

#include <cmath>
#include <queue>
#include <iostream>
#include <fstream>

Eigen::MatrixXi DKD::maxpool_detect_keypoints(const uint8_t* scores_map, int H, int W){
    // Map existing scores data the the eigen matrix
    Eigen::Map<const Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> scores(scores_map, H, W);
    
    int full_padding = m_padding + m_radius;
    int h_start = full_padding;
    int w_start = full_padding;
    int h_stop = H - full_padding;
    int w_stop = W - full_padding;

    auto maxpooled = maxpool2d(
        scores,
        h_start, w_start,
        h_stop, w_stop, 
        m_radius, 1
    );   



    Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic> max_mask = (scores.array() == maxpooled.array()).matrix();
    Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> int_mask = max_mask.cast<uint8_t>();

    // static int frame_count = 0;
    // do {// Write keypoints to file
    
    //     std::string scrs_path = "mxpl/mxpl" + std::to_string(frame_count++) + ".hm";
    //     std::ofstream scores_file(scrs_path, std::ios::binary | std::ios::out);
    //     if (!scores_file) {
    //         std::cerr << "Failed to open output file for keypoints." << std::endl;
    //         continue;
    //     }
    //     scores_file.write(
    //         reinterpret_cast<const char*>(int_mask.data()), 
    //         sizeof(int_mask[0]) * int_mask.size()
    //     );
    //     scores_file.close();
    // } while(0);
    
    auto cmp = [&scores](const Eigen::Vector2i& a, const Eigen::Vector2i& b) {
        return scores(a.y(), a.x()) > scores(b.y(), b.x());
    };

    std::priority_queue<Eigen::Vector2i, std::vector<Eigen::Vector2i>, decltype(cmp)> pq(cmp);
    
    // Extarct keypoints from the score map using maximum mask
    for (int i = h_start; i < h_stop; ++i) {
        for (int j = w_start; j < w_stop; ++j) {
            if (max_mask(i, j) && scores(i, j) > 127) {
                pq.push(Eigen::Vector2i(j, i));
                if (pq.size() > m_top_k) {
                    pq.pop();
                }
            }
        }
    }
    
    // Select top k keypoints by their value in the score map
    Eigen::Matrix<int, Eigen::Dynamic, 2, Eigen::RowMajor> keypoints(m_top_k, 2);
    for (int i = m_top_k - 1; i >= 0; --i) {
        if (!pq.empty()) {
            keypoints.row(i) = pq.top();
            pq.pop();
        }
    }
    
    return keypoints;
}

Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> DKD::sample_descriptors(const uint8_t* descriptor_map, const Eigen::MatrixXi& kpts, int D, int H, int W) {
// Eigen::MatrixXf DKD::sample_descriptors(const uint8_t* descriptor_map, const Eigen::MatrixXi& kpts, int D, int H, int W) {
    Eigen::Map<const Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> descriptors_reshaped(descriptor_map, D, H * W);
    Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> descriptors(kpts.rows(), D);

    for (int i = 0; i < kpts.rows(); ++i) {
        int x = kpts(i, 0);
        int y = kpts(i, 1);
        
        // FIXME: remove in release, cause out-of-bounds should never hapen if the code is right
        // Add boundary checks
        if (x >= 0 && x < W && y >= 0 && y < H) {
            descriptors.row(i) = descriptors_reshaped.col(y * W + x).transpose();
        } else {
            std::cout << "Out of bound detected: " << x << ", " << y << std::endl;
            // Handle out-of-bounds access, e.g., set to zero or some default value
            descriptors.row(i).setZero();
        }
    }

    return descriptors;
}

void DKD::run(const uint8_t* scores_map, const uint8_t* descriptor_map,
        Eigen::MatrixXi& keypoints, Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>& descriptors, 
        // Eigen::MatrixXi& keypoints, Eigen::MatrixXf& descriptors, 
        size_t D, size_t H, size_t W){

    // Extract keypoints' UVs
    keypoints = maxpool_detect_keypoints(scores_map, H, W);

    // Sample descriptors using UVs
    descriptors = sample_descriptors(descriptor_map, keypoints, D, H, W);
    //descriptors.rowwise().normalize();
}


/*
* Helper methods
*/

Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> 
DKD::maxpool2d(const Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>& input, 
               int h_start, int w_start, int h_stop, int w_stop, int radius, int stride) {
    // Initialize output matrix with zeros
    Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> output = 
        Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>::Zero(input.rows(), input.cols());
    
    int kernel_size = 2 * radius + 1;

    for (int i = h_start; i < h_stop; i += stride) {
        for (int j = w_start; j < w_stop; j += stride) {
            // Extract kernel block
            Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> kernel = 
                input.block(i - radius, j - radius, kernel_size, kernel_size);

            output(i, j) = kernel.maxCoeff();
        }
    }

    return output;
}