/************************************************************************
*
*   Copyright (C), 2012-2020, GTBS. Co., Ltd.
*
*   软件模块 ：基础框架
*   功   能	：USB摄像头实现类（基于 OpenCV）
*
*************************************************************************/
#include "usb_camera.h"
#include <iostream>
#include <memory>

// 构造函数
USBCamera::USBCamera(int camera_index, const cv::Size& resolution, int fps)
    : BaseCamera(std::to_string(camera_index))
    , camera_index_(camera_index)
    , resolution_(resolution)
    , fps_(fps)
    , cap_(nullptr) 
{

}

// 析构函数（基类会调用 closeCamera，这里无需额外操作）
USBCamera::~USBCamera() = default;

// 初始化相机
bool USBCamera::initCamera() {
    if (isInitialized()) {
        return true;
    }

    cap_ = std::make_unique<cv::VideoCapture>(camera_index_);
    if (!cap_ || !cap_->isOpened()) {
        std::cerr << "[ERROR] Failed to open USB camera " << getCameraId() << std::endl;
        cap_.reset();
        return false;
    }

    // 设置参数
    cap_->set(cv::CAP_PROP_FRAME_WIDTH, static_cast<double>(resolution_.width));
    cap_->set(cv::CAP_PROP_FRAME_HEIGHT, static_cast<double>(resolution_.height));
    cap_->set(cv::CAP_PROP_FPS, static_cast<double>(fps_));

    // 设置曝光（仅当非自动时）
    if (exposure_ >= 0) {
        cap_->set(cv::CAP_PROP_EXPOSURE, static_cast<double>(exposure_));
    }

    setInitialized(true);
    std::cout << "[INFO] USB camera " << getCameraId() << " initialized!" << std::endl;
    return true;
}

// 采集单帧
cv::Mat USBCamera::captureSingleFrame() {
    if (!cap_ || !cap_->isOpened() || !isInitialized()) {
        return cv::Mat();
    }

    cv::Mat frame;
    *cap_ >> frame;
    return frame;
}

// 关闭相机
void USBCamera::closeCamera() {
    if (cap_) {
        cap_->release();
        cap_.reset();
    }
    setInitialized(false);
    std::cout << "[INFO] USB camera " << getCameraId() << " closed!" << std::endl;
}

// 设置曝光
void USBCamera::setExposure(int exposure) {
    exposure_ = exposure;
    if (cap_ && isInitialized()) {
        cap_->set(cv::CAP_PROP_EXPOSURE, static_cast<double>(exposure_));
    }
}

/*************************************************************************
* 改动历史纪录：
Revision 1.0, 2026-01-20, lium
describe: 初始创建.
*************************************************************************/