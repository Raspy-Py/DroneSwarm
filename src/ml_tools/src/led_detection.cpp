#include "led_detection.hpp"

std::vector<std::pair<float, float>> LedDetector::detect(const Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> &r_channel,
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
    if (debug)
    {
        std::cout << "\nInput Matrices:" << std::endl;
        std::cout << "\nR channel:" << std::endl
                    << r_channel;
        std::cout << "\nG channel:" << std::endl
                    << g_channel;
        std::cout << "\nB channel:" << std::endl
                    << b_channel;
        print_scores(red_scores, width, height, "Red");
        print_scores(green_scores, width, height, "Green");
        print_scores(blue_scores, width, height, "Blue");
    }

    Eigen::Index red_idx, green_idx, blue_idx;
    red_scores.minCoeff(&red_idx);
    green_scores.minCoeff(&green_idx);
    blue_scores.minCoeff(&blue_idx);

    std::pair<float, float> redp = {red_idx % width, red_idx / width},
                            greenp = {green_idx % width, green_idx / width},
                            bluep = {blue_idx % width, blue_idx / width};
    return std::vector<std::pair<float, float>>{redp, greenp, bluep};
}


void LedDetector::print_scores(const Eigen::Matrix<float, 1, Eigen::Dynamic, Eigen::RowMajor> &scores,
                    int width, int height, const std::string &name)
{
    std::cout << "\n" << name << " scores matrix:" << std::endl;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x)
            std::cout << std::fixed << std::setprecision(3) << scores(y * width + x) << "\t";
        std::cout << std::endl;
    }
    Eigen::Index min_idx;
    float min_val = scores.minCoeff(&min_idx);
    int min_x = min_idx % width;
    int min_y = min_idx / width;
    std::cout << "Minimum score: " << min_val << " at position (" << min_x << ", " << min_y << ")\n";
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
    
    std::vector<std::pair<float, float>> results = detect(red, green, blue, false);
    
    std::cout << "\nLED Detection Results:" << std::endl;
    std::cout << "Red LED found at: (" << results[0].first << ", " << results[0].second << ")" << std::endl;
    std::cout << "Green LED found at: (" << results[1].first << ", " << results[1].second << ")" << std::endl;
    std::cout << "Blue LED found at: (" << results[2].first << ", " << results[2].second << ")" << std::endl;
}
