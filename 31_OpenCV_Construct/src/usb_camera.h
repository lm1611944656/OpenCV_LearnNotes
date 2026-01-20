/************************************************************************
*
*   Copyright (C), 2012-2020, GTBS. Co., Ltd.
*
*   软件模块 ：基础框架
*   功   能	：USB摄像头实现类（基于 OpenCV）
*
*************************************************************************/
#ifndef __USB_CAMERA_H__
#define __USB_CAMERA_H__

#include "base_camera.h" 
#include <opencv2/opencv.hpp>
#include <memory>

class USBCamera : public BaseCamera {
public:
    // 构造函数：支持默认分辨率和帧率
    explicit USBCamera(int camera_index,
                       const cv::Size& resolution = cv::Size(640, 480),
                       int fps = 30);

    ~USBCamera() override;

    // 实现纯虚函数
    bool initCamera() override;
    cv::Mat captureSingleFrame() override;

    // 重写虚函数
    void closeCamera() override;

    // 额外接口
    void setExposure(int exposure);

private:
    int camera_index_;
    cv::Size resolution_;
    int fps_;
    int exposure_ = -1;  // -1 表示自动曝光
    std::unique_ptr<cv::VideoCapture> cap_;
};

#endif // __USB_CAMERA_H__

/*************************************************************************
* 改动历史纪录：
Revision 1.0, 2026-01-20, lium
describe: 初始创建.
*************************************************************************/