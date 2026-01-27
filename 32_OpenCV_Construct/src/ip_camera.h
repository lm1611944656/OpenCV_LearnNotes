/************************************************************************
*
*   Copyright (C), 2012-2020, GTBS. Co., Ltd.
*
*   软件模块 ：基础框架
*   功   能	：网络摄像头实现类（基于 RTSP）
*
*************************************************************************/
#ifndef __IP_CAMERA_H__
#define __IP_CAMERA_H__

#include "base_camera.h"
#include <opencv2/opencv.hpp>
#include <string>
#include <memory>

class IPCamera : public BaseCamera {
public:
    // 构造函数：支持自定义 RTSP 流路径和端口
    IPCamera(const std::string& camera_id,
             const std::string& ip,
             const std::string& username,
             const std::string& password,
             const std::string& stream_path = "stream1",
             int port = 554);

    ~IPCamera() override;

    // 实现纯虚函数
    bool initCamera() override;
    cv::Mat captureSingleFrame() override;

    // 重写虚函数
    void closeCamera() override;

private:
    std::string rtsp_url_;
    std::string ip_;
    std::string username_;
    std::string password_;
    int port_;
    std::string stream_path_;
    std::unique_ptr<cv::VideoCapture> cap_;

};

#endif // __IP_CAMERA_H__
/*************************************************************************
* 改动历史纪录：
Revision 1.0, 2026-01-20, lium
describe: 初始创建.
*************************************************************************/