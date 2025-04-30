#include <opencv2/opencv.hpp>
#include <iostream>
#include <memory>
#include "ImageCoordinates.h"

const std::string imgPath = "data/srcImg.png";

int main(int argc, char **argv) {
    try {
        std::shared_ptr<CImgCoordinates> imgCoordinate = std::make_shared<CImgCoordinates>(imgPath);

        // 创建窗口并绑定鼠标回调函数
        cv::namedWindow("image");
        cv::setMouseCallback("image", &CImgCoordinates::getImgCoordinates, imgCoordinate.get());

        // 显示图片
        cv::imshow("image", imgCoordinate->getImage());  // 注意：我们需要添加一个 getImage 方法

        // 等待用户按键操作
        while (true) {
            char key = cv::waitKey(5) & 0xFF;
            if (key == 'q') {  // 按下 'q' 键退出
                break;
            }
        }

        // 关闭所有窗口
        cv::destroyAllWindows();
    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
        return -1;
    }

    return 0;
}