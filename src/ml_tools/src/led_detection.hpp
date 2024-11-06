#ifndef LED_DETECTOR_H
#define LED_DETECTOR_H

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
};

#endif