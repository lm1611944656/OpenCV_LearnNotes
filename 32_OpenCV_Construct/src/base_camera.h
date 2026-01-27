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

/**图像帧结构体（包含相机ID、时间戳、图像)*/
struct FrameData {
	std::string camera_id;          // 相机唯一标识
	double timestamp;               // 采集时间戳(秒)
	cv::Mat frame;                  // 图像数据（深拷贝避免悬空引用）

	// 构造函数（深拷贝Mat）
	FrameData() {};

	FrameData(std::string id, double ts, const cv::Mat& img)
		: camera_id(std::move(id)), timestamp(ts), frame(img.clone()) {}
};

/**性能监控指标(帧率统计、队列积压告警、丢帧计数) */
struct CameraMetrics {
    double current_fps = 0.0;          // 当前帧率（近1秒平均）
    size_t queue_size = 0;             // 当前队列长度
    size_t dropped_frames = 0;         // 累计丢帧数
    size_t total_captured = 0;         // 累计采集帧数（用于计算丢帧率）

    // 格式化输出性能监控指标
    std::string toString() const {
        char buf[256];
        std::snprintf(buf, sizeof(buf),
            "FPS: %.2f | Queue: %zu/%zu | Dropped: %zu (%.2f%%)",
            current_fps,
            queue_size, MAX_QUEUE_SIZE,
            dropped_frames,
            total_captured > 0 ? (dropped_frames * 100.0 / total_captured) : 0.0
        );
        return std::string(buf);
    }
};

class BaseCamera {
public:
    explicit BaseCamera(std::string camera_id);
    virtual ~BaseCamera();
 
    /**禁止拷贝，允许移动*/
    BaseCamera(const BaseCamera&) = delete;
    BaseCamera& operator=(const BaseCamera&) = delete;
    BaseCamera(BaseCamera&&) = default;
    BaseCamera& operator=(BaseCamera&&) = default;

    /**纯虚函数：子类必须实现*/
    virtual bool initCamera() = 0;
    virtual cv::Mat captureSingleFrame() = 0;

    /**虚函数：可选重写*/
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

    // 获取当前性能指标（线程安全）
    CameraMetrics getMetrics() const;

protected:
    void setInitialized(bool status);
    void captureLoop(); // 采集线程主循环

    std::string camera_id_;
    std::atomic<bool> initialized_;
    std::atomic<bool> capturing_;
    ThreadSafeQueue<FrameData> frame_queue_;
    std::thread capture_thread_;

    // 新增性能监控成员
    mutable std::mutex metrics_mtx_;                          // 指标保护锁
    CameraMetrics metrics_;                                   // 当前性能指标
    std::atomic<size_t> local_dropped_count_{0};              // 本相机累计丢帧数（原子更安全）
    size_t frames_in_interval_ = 0;                           // 当前统计窗口内的帧数
    std::chrono::steady_clock::time_point last_fps_update_ = std::chrono::steady_clock::now();    // 上次FPS更新时间
    static constexpr double FPS_INTERVAL_SEC = 1.0;           // FPS统计间隔（秒）
};

#endif // __BASE_CAMERA_H__

/*************************************************************************
* 改动历史纪录：
Revision 1.0, 2026-01-20, lium
describe: 初始创建.
*************************************************************************/