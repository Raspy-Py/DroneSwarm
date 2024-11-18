#pragma once
#include <Eigen/Dense>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <utility>

class LedDetector
{
public:
    void print_scores(const Eigen::Matrix<float, 1, Eigen::Dynamic, Eigen::RowMajor> &scores,
                      int width, int height, const std::string &name);

    std::vector<std::pair<float, float>> detect(const Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> &r_channel,
                                               const Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> &g_channel,
                                               const Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> &b_channel,
                                               bool debug);
    std::tuple<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>,
                Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>,
                Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> 
                    mapImageToEigen(const void* imageData, int width, int height);
    void map_test(const void* imageData, int width, int height);
};
