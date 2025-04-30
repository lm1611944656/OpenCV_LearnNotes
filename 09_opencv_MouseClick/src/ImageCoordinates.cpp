#include "ImageCoordinates.h"
#include <iostream>

CImgCoordinates::CImgCoordinates(const std::string &imgPath)
    : _imgPath(imgPath)
{
    imRead(_imgPath);
}

CImgCoordinates::~CImgCoordinates() {}

void CImgCoordinates::getImgCoordinates(int event, int x, int y, int flags, void* userdata)
{
    CImgCoordinates* instance = static_cast<CImgCoordinates*>(userdata);
    if (event == cv::EVENT_LBUTTONDOWN) {
        // 打印点击的坐标
        std::cout << "Clicked at: (" << x << ", " << y << ")" << std::endl;

        // 在点击位置绘制一个绿色圆点
        cv::circle(instance->srcImg, cv::Point(x, y), 2, cv::Scalar(0, 255, 0), -1);

        // 将坐标转换为字符串并绘制到图像上
        std::string xy = std::to_string(x) + "," + std::to_string(y);
        cv::putText(instance->srcImg, xy, cv::Point(x, y),
                    cv::FONT_HERSHEY_PLAIN, 3.0, cv::Scalar(0, 255, 0), 1);

        // 更新窗口以显示变化
        cv::imshow("image", instance->srcImg);
    }
}

void CImgCoordinates::imRead(const std::string &imgPath)
{
    srcImg = cv::imread(imgPath, cv::IMREAD_COLOR);
    if (srcImg.empty()) {
        std::cerr << "Error: Could not load image from path: " << imgPath << std::endl;
        throw std::invalid_argument("图像路径无效或图像加载失败!");
    }
}

const cv::Mat& CImgCoordinates::getImage() const {
    return srcImg;
}