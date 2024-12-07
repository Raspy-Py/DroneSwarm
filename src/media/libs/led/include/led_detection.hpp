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

    std::vector<std::pair<int, int>> detect(const Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> &r_channel,
                                               const Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> &g_channel,
                                               const Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> &b_channel,
                                               bool debug);

    std::tuple<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>,
                Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>,
                Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> 
                    mapImageToEigen(const void* imageData, int width, int height);

    void map_test(const void* imageData, int width, int height);

    double calculate_angle(const std::pair<int, int>& pixel_1, 
                       const std::pair<int, int>& pixel_2, 
                       double fx, double fy, double cx, double cy);
    std::vector<double> calculate_delta(double x1, double x2, double x3, 
                                    double cosa, double cosb, double cosc, 
                                    double ncosa, double ncosb, double ncosc);
};
