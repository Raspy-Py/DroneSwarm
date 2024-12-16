#include "led_detection.hpp"

std::vector<std::pair<int, int>> LedDetector::detect(const Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> &r_channel,
                                            const Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> &g_channel,
                                            const Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> &b_channel,
                                            bool debug = true)
{
    int height = r_channel.rows();
    int width = r_channel.cols();

    Eigen::Map<const Eigen::Matrix<float, 1, Eigen::Dynamic, Eigen::RowMajor>> r_flat(r_channel.data(), 1, r_channel.size());
    Eigen::Map<const Eigen::Matrix<float, 1, Eigen::Dynamic, Eigen::RowMajor>> g_flat(g_channel.data(), 1, g_channel.size());
    Eigen::Map<const Eigen::Matrix<float, 1, Eigen::Dynamic, Eigen::RowMajor>> b_flat(b_channel.data(), 1, b_channel.size());

    Eigen::Matrix<float, 1, Eigen::Dynamic, Eigen::RowMajor> red_scores = -r_flat.array() +
                                                                            g_flat.array() + b_flat.array();
    Eigen::Matrix<float, 1, Eigen::Dynamic, Eigen::RowMajor> green_scores = -g_flat.array() +
                                                                            r_flat.array() + b_flat.array();
    Eigen::Matrix<float, 1, Eigen::Dynamic, Eigen::RowMajor> blue_scores = -b_flat.array() +
                                                                            r_flat.array() + g_flat.array();
    Eigen::Index red_idx, green_idx, blue_idx;
    red_scores.minCoeff(&red_idx);
    green_scores.minCoeff(&green_idx);
    blue_scores.minCoeff(&blue_idx);

    std::pair<float, float> redp = {red_idx % width, red_idx / width},
                            greenp = {green_idx % width, green_idx / width},
                            bluep = {blue_idx % width, blue_idx / width};
    return std::vector<std::pair<int, int>>{redp, greenp, bluep};
}


std::tuple<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>,
Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>,
Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> 
LedDetector::mapImageToEigen(const void* imageData, int width, int height) {
    const uint8_t* data = static_cast<const uint8_t*>(imageData);

    std::vector<uint8_t> aligned_buffer(width * height * 3);
    memcpy(aligned_buffer.data(), data, width * height * 3);

    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> 
        red(height, width),
        green(height, width),
        blue(height, width);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int pixel_idx = (y * width + x) * 3;
            blue(y, x) = aligned_buffer[pixel_idx] / 255.0f;
            green(y, x) = aligned_buffer[pixel_idx + 1] / 255.0f;
            red(y, x) = aligned_buffer[pixel_idx + 2] / 255.0f;
        }
    }
    return std::make_tuple(red, green, blue);
}


void LedDetector::map_test(const void* imageData, int width, int height) {
    auto result = mapImageToEigen(imageData, width, height);
    auto red = std::get<0>(result);
    auto green = std::get<1>(result);
    auto blue = std::get<2>(result);
    
    std::vector<std::pair<int, int>> results = detect(red, green, blue, false);
    
    std::cout << "\nLED Detection Results:" << std::endl;
    std::cout << "Red LED found at: (" << results[0].first << ", " << results[0].second << ")" << std::endl;
    std::cout << "Green LED found at: (" << results[1].first << ", " << results[1].second << ")" << std::endl;
    std::cout << "Blue LED found at: (" << results[2].first << ", " << results[2].second << ")" << std::endl;
}

float calculateDistance(const std::vector<std::pair<int, int>>& led_coords) {
    Eigen::Matrix<float, 3, 3> led_coords_homogeneous;
    for (int i = 0; i < 3; ++i) {
        led_coords_homogeneous(i, 0) = led_coords[i].first;  // x-coordinate
        led_coords_homogeneous(i, 1) = led_coords[i].second; // y-coordinate
        led_coords_homogeneous(i, 2) = 1.0f;                // homogeneous coordinate
    }
    Eigen::Matrix<float, 3, 3> rays = intrinsics.inverse() * led_coords_homogeneous.transpose();
    auto objectiveFunction = [&](const Eigen::Vector3f& lambdas) {
        Eigen::Vector3f p1 = lambdas(0) * rays.col(0);
        Eigen::Vector3f p2 = lambdas(1) * rays.col(1);
        Eigen::Vector3f p3 = lambdas(2) * rays.col(2);

        float dist12 = (p1 - p2).norm();
        float dist13 = (p1 - p3).norm();
        float dist23 = (p2 - p3).norm();
        return std::pow(dist12 - d12, 2) + std::pow(dist13 - d13, 2) + std::pow(dist23 - d23, 2);
    };

    Eigen::Vector3f lambdas = Eigen::Vector3f::Ones();
    const float epsilon = 1e-6;
    const int max_iterations = 1000;
    float learning_rate = 1e-2;
    for (int iter = 0; iter < max_iterations; ++iter) {
        Eigen::Vector3f gradient;
        for (int i = 0; i < 3; ++i) {
            Eigen::Vector3f lambdas_up = lambdas, lambdas_down = lambdas;
            lambdas_up(i) += epsilon;
            lambdas_down(i) -= epsilon;
            gradient(i) = (objectiveFunction(lambdas_up) - objectiveFunction(lambdas_down)) / (2 * epsilon);
        }
        lambdas -= learning_rate * gradient;
        if (gradient.norm() < epsilon) break;
    }

    Eigen::Vector3f p1 = lambdas(0) * rays.col(0);
    Eigen::Vector3f p2 = lambdas(1) * rays.col(1);
    Eigen::Vector3f p3 = lambdas(2) * rays.col(2);

    Eigen::Vector3f centroid = (p1 + p2 + p3) / 3.0f;

    return centroid.norm();
}