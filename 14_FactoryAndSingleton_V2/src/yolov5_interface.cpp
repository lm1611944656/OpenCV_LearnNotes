#include "yolov5_interface.h"


// YOLOv5 的 infer 方法实现
void YOLOv5::infer(const std::string image_path) {
    // m_image = cv::imread(image_path);
    // if (m_image.empty()) {
    //     std::cerr << "Error: Unable to load image!" << std::endl;
    //     return;
    // }
    std::cout << "读取图像成功：" << image_path << std::endl;
    pre_process();
    process();
    post_process();
}

// YOLOv5 的 post_process 方法实现
void YOLOv5::post_process() {
    std::cout << "Post-processing..." << std::endl;
}