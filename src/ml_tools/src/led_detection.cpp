#include "led_detection.hpp"

void LedDetector::print_scores(const Eigen::Matrix<float, 1, Eigen::Dynamic, Eigen::RowMajor> &scores,
                    int width, int height, const std::string &name)
{
    std::cout << "\n"
                << name << " scores matrix:" << std::endl;
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            std::cout << std::fixed << std::setprecision(3) << scores(y * width + x) << "\t";
        }
        std::cout << std::endl;
    }
    Eigen::Index min_idx;
    float min_val = scores.minCoeff(&min_idx);
    int min_x = min_idx % width;
    int min_y = min_idx / width;
    std::cout << "Minimum score: " << min_val << " at position (" << min_x << ", " << min_y << ")\n";
}

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

    std::pair<float, float> redp = {red_idx / width, red_idx % width},
                            greenp = {green_idx / width, green_idx % width},
                            bluep = {blue_idx / width, blue_idx % width};
    return std::vector<std::pair<float, float>>{redp, greenp, bluep};
}


int main()
{
    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> R(4, 4);
    R << 1.0f, 0.5f, 0.5f, 0.7f,
        1.0f, 0.3f, 0.3f, 0.3f,
        0.2f, 0.2f, 0.2f, 0.2f,
        1.0f, 1.0f, 1.0f, 1.0f;

    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> B(4, 4);
    B << 1.0f, 1.0f, 1.0f, 1.0f,
        0.1f, 0.9f, 0.9f, 0.9f,
        0.9f, 0.9f, 0.89f, 0.9f,
        1.0f, 1.0f, 1.0f, 1.0f;

    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> G(4, 4);
    G << 1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f;

    LedDetector detector;
    std::vector<std::pair<float, float>> results = detector.detect(R, G, B);

    std::cout << "\nFinal Results:" << std::endl;
    std::cout << "Red LED found at: (" << results[0].first << ", " << results[0].second << ")" << std::endl;
    std::cout << "Green LED found at: (" << results[1].first << ", " << results[1].second << ")" << std::endl;
    std::cout << "Blue LED found at: (" << results[2].first << ", " << results[2].second << ")" << std::endl;
    return 0;
}