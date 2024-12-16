#ifndef ML_TOOLS_ALIKE_DKD_H
#define ML_TOOLS_ALIKE_DKD_H

#include <Eigen/Dense>
#include <vector>
#include <algorithm>


class DKD {
public:
    DKD(int top_k = 500, int radius = 4, int padding = 2)
        : m_top_k(top_k), m_radius(radius), m_padding(padding) {}
    
    void run(const uint8_t* scores_map, const uint8_t* descriptor_map,
        Eigen::MatrixXi& keypoints, Eigen::MatrixXf& descriptors, 
        size_t D, size_t H, size_t W);

private:
    // From a pretty random number of detected keypoints only top k
    // with the most intensity will be selected
    int m_top_k;

    // Radius of the local region in which we seek for the maximum intensity
    // points on the score map
    int m_radius;

    // ALike tends to give false postives close to borders. Thus we skip
    // several rows in the score map during keypoint detection
    int m_padding;
    
private:
    /// @brief Extract keypoints from the the score map using simple nms via single maxpool run
    /// @return Matrix (num_pts x 2) of extracted keypoints
    Eigen::MatrixXi maxpool_detect_keypoints(const uint8_t* scores_map, int H, int W);

    /// @brief Using keypoints' UVs sample descriptor vectors from the descriptor map
    /// @return Matrix (num_pts x channels) of sampled descriptors
    Eigen::MatrixXf sample_descriptors(const uint8_t* descriptor_map, const Eigen::MatrixXi& kpts, int D, int H, int W);

private:
    // Super specific maxpool implementation optimized for dkd
    static Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> 
    maxpool2d(const Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>& input, 
               int h_start, int w_start, int h_stop, int w_stop, 
               int radius, int stride = 1);
};

#endif // ML_TOOLS_ALIKE_DKD_H
