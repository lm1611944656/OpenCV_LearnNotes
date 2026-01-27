/************************************************************************
*
*   Copyright (C), 2012-2020, GTBS. Co., Ltd.
*
*   软件模块 ：基础框架
*   功   能	：网络摄像头实现类（基于 RTSP）
*
*************************************************************************/
#include "ip_camera.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <opencv2/opencv.hpp>  // 确保 VideoCapture 可用

// 构造函数
IPCamera::IPCamera(const std::string& camera_id,
                   const std::string& ip,
                   const std::string& username,
                   const std::string& password,
                   const std::string& stream_path,
                   int port)
    : BaseCamera(camera_id)
    , rtsp_url_("rtsp://" + username + ":" + password + "@" + ip + ":" + std::to_string(port) + "/" + stream_path)
    , ip_(ip)
    , username_(username)
    , password_(password)
    , port_(port)
    , stream_path_(stream_path)
    , cap_(nullptr)
{}

// 析构函数
IPCamera::~IPCamera() = default;

// 初始化 RTSP 相机
bool IPCamera::initCamera() {
    if (isInitialized()) {
        return true;
    }

    std::cout << "[INFO] Trying to open RTSP stream: " << rtsp_url_ << std::endl;

    int retry = 5;
    while (retry-- > 0) {
        cap_ = std::make_unique<cv::VideoCapture>(rtsp_url_);
        if (cap_ && cap_->isOpened()) {
            break;
        }
        cap_.reset();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    if (!cap_ || !cap_->isOpened()) {
        std::cerr << "[ERROR] Failed to open RTSP camera " << getCameraId()
                  << " at " << rtsp_url_ << std::endl;
        return false;
    }

    // 设置缓冲区大小为1，降低延迟（对实时流很重要）
    cap_->set(cv::CAP_PROP_BUFFERSIZE, 1.0);

    setInitialized(true);
    std::cout << "[INFO] RTSP camera " << getCameraId() << " initialized!" << std::endl;
    return true;
}

// 采集单帧
cv::Mat IPCamera::captureSingleFrame() 
{
    if (!cap_ || !cap_->isOpened() || !isInitialized()) {
        return cv::Mat();
    }

    // 开始采集帧
    cv::Mat frame;
    *cap_ >> frame;  // 可能阻塞，但由采集线程和队列超时机制兜底
    return frame;
}

// 关闭相机
void IPCamera::closeCamera() {
    if (cap_) {
        cap_->release();
        cap_.reset();
    }
    setInitialized(false);
    std::cout << "[INFO] RTSP camera " << getCameraId() << " closed!" << std::endl;
}

/*************************************************************************
* 改动历史纪录：
Revision 1.0, 2026-01-20, lium
describe: 初始创建.
*************************************************************************/