/************************************************************************
*
*   Copyright (C), 2012-2020, GTBS. Co., Ltd.
*
*   软件模块 ：基础框架
*   功   能	：相机抽象基类
*
*************************************************************************/
#ifndef __BASE_CAMERA_H__
#define __BASE_CAMERA_H__

#include <string>
#include <thread>
#include <atomic>
#include <memory>
#include <iostream>
#include <chrono>
#include <opencv2/opencv.hpp>

#include "thread_safe_queue.h" 

// 帧数据结构体（包含相机ID、时间戳、图像）
struct FrameData {
	std::string camera_id;          // 相机唯一标识
	double timestamp;               // 采集时间戳(秒)
	cv::Mat frame;                  // 图像数据（深拷贝避免悬空引用）

	// 构造函数（深拷贝Mat）
	FrameData() {};

	FrameData(std::string id, double ts, const cv::Mat& img)
		: camera_id(std::move(id)), timestamp(ts), frame(img.clone()) {}
};

class BaseCamera {
public:
    explicit BaseCamera(std::string camera_id);
    virtual ~BaseCamera();

    // 禁止拷贝，允许移动
    BaseCamera(const BaseCamera&) = delete;
    BaseCamera& operator=(const BaseCamera&) = delete;
    BaseCamera(BaseCamera&&) = default;
    BaseCamera& operator=(BaseCamera&&) = default;

    // 纯虚函数：子类必须实现
    virtual bool initCamera() = 0;
    virtual cv::Mat captureSingleFrame() = 0;

    // 虚函数：可选重写
    virtual void closeCamera();

    // 控制接口
    bool startCapture();
    void stopCapture();

    // 数据获取
    FrameData getFrame(int timeout_ms = QUEUE_TIMEOUT_MS);
    void clearQueue();

    // 查询状态
    std::string getCameraId() const;
    bool isInitialized() const;
    bool isCapturing() const;

protected:
    void setInitialized(bool status);
    void captureLoop(); // 采集线程主循环

    std::string camera_id_;
    std::atomic<bool> initialized_;
    std::atomic<bool> capturing_;
    ThreadSafeQueue<FrameData> frame_queue_;
    std::thread capture_thread_;
};

#endif // __BASE_CAMERA_H__

/*************************************************************************
* 改动历史纪录：
Revision 1.0, 2026-01-20, lium
describe: 初始创建.
*************************************************************************/