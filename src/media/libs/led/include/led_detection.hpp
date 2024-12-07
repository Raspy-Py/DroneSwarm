#pragma once
#include <Eigen/Dense>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <utility>

constexpr float d12 = 0.15f;  // Distance between LED 1 and 2
constexpr float d13 = 0.1f; // Distance between LED 1 and 3
constexpr float d23 = 0.1f;  // Distance between LED 2 and 3

const Eigen::Matrix3f intrinsics = (Eigen::Matrix3f() << 
        323.04f, 0.0f, 250.84f,
        0.0f, 321.8f, 141.83f,
        0.0f, 0.0f, 1.0f).finished();

class LedDetector
{
public:
    std::vector<std::pair<int, int>> detect(const Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> &r_channel,
                                               const Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> &g_channel,
                                               const Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> &b_channel,
                                               bool debug);

    std::tuple<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>,
                Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>,
                Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> 
                    mapImageToEigen(const void* imageData, int width, int height);

    void map_test(const void* imageData, int width, int height);
};

float calculateDistance(const std::vector<std::pair<int, int>>& led_coords);