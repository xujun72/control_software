#include "config.hpp"
#include <iostream>
#include <opencv2/opencv.hpp>

void annotate_and_save_faults(
    const std::string& input_image_path,
    const std::vector<Recv_picinfo>& faults,
    const std::string& output_image_path
) {
    // 加载原图
    cv::Mat image = cv::imread(input_image_path);
    if (image.empty()) {
        std::cerr << "无法加载图像: " << input_image_path << std::endl;
        return;
    }

    // 遍历绘制每个故障框
    for (size_t i = 0; i < faults.size(); ++i) {
        const auto& info = faults[i];

        // 绘制矩形
        cv::Rect box(info.x, info.y, info.w, info.h);
        cv::Scalar color(0, 0, 255); // 红色
        cv::rectangle(image, box, color, 2);

        // 文本标签
        std::string label = "P:" + std::to_string(info.part_label) +
                            " F:" + std::to_string(info.fault_label) +
                            " C:" + std::to_string(info.conf);
        int font = cv::FONT_HERSHEY_SIMPLEX;
        cv::putText(image, label, cv::Point(info.x, info.y - 5), font, 0.5, color, 1);
    }

    // 保存结果图像
    if (!cv::imwrite(output_image_path, image)) {
        std::cerr << "无法保存图像: " << output_image_path << std::endl;
    } else {
        std::cout << "标注图已保存: " << output_image_path << std::endl;
    }
}