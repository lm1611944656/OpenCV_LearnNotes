/************************************************************************
*
*   Copyright (C), 2012-2020, GTBS. Co., Ltd.
*
*   软件模块 ：基础框架
*   功   能	：相机抽象基类源文件
*
*************************************************************************/
#include "base_camera.h"
#include <stdexcept>
#include <cerrno>

// 构造函数
BaseCamera::BaseCamera(std::string camera_id)
    : camera_id_(std::move(camera_id))
    , initialized_(false)
    , capturing_(false) {}

// 析构函数
BaseCamera::~BaseCamera() {
    stopCapture();
    closeCamera();
}

// 默认 closeCamera 实现
void BaseCamera::closeCamera() {
    std::cout << "[INFO] BaseCamera " << camera_id_ << " closed (default impl)!" << std::endl;
}

// 启动采集线程
bool BaseCamera::startCapture() {
    if (!initialized_) {
        std::cerr << "[ERROR] Camera " << camera_id_ << " not initialized!" << std::endl;
        return false;
    }

    if (capturing_) {
        std::cerr << "[WARNING] Camera " << camera_id_ << " is already capturing!" << std::endl;
        return true;
    }

    capturing_ = true;
    capture_thread_ = std::thread(&BaseCamera::captureLoop, this);
    capture_thread_.detach();
    std::cout << "[INFO] Camera " << camera_id_ << " capture thread started!" << std::endl;
    return true;
}

// 停止采集
void BaseCamera::stopCapture() {
    if (!capturing_) {
        return;
    }

    capturing_ = false;
    // 给线程一点时间退出（注意：detach 线程无法 join，这里只是等待逻辑退出）
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << "[INFO] Camera " << camera_id_ << " capture thread stopped!" << std::endl;
}

// 获取帧（带超时）
FrameData BaseCamera::getFrame(int timeout_ms) {
    FrameData frame_data("", 0.0, cv::Mat());
    if (frame_queue_.pop(frame_data, timeout_ms)) {
        return frame_data;
    }
    return FrameData(); // 返回空帧（默认构造）
}

// 清空队列
void BaseCamera::clearQueue() {
    frame_queue_.clear();
}

// Getter
std::string BaseCamera::getCameraId() const 
{ 
    return camera_id_; 
}

bool BaseCamera::isInitialized() const 
{ 
    return initialized_; 
}

bool BaseCamera::isCapturing() const 
{ 
    return capturing_; 
}

CameraMetrics BaseCamera::getMetrics() const {
    std::lock_guard<std::mutex> lock(metrics_mtx_);
    return metrics_;
}

// Protected
void BaseCamera::setInitialized(bool status) {
    initialized_ = status;
}

// 采集循环（线程函数）
void BaseCamera::captureLoop() {
    while (capturing_) {
        try {
            cv::Mat frame = captureSingleFrame();
            if (frame.empty()) {
                std::this_thread::sleep_for(std::chrono::microseconds(1000));
                continue;
            }

            double timestamp = std::chrono::duration_cast<std::chrono::duration<double>>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();

            FrameData frame_data(camera_id_, timestamp, frame);

            // 入队（可能丢帧）
            if (!frame_queue_.push(frame_data)) {
                std::cerr << "[WARNING] Camera " << camera_id_ << " queue push timeout!" << std::endl;
            }

            // 更新性能监控指标
            // 获取本次循环中新增的丢帧数
            size_t current_dropped = frame_queue_.getAndResetDroppedCount();
            local_dropped_count_ += current_dropped;
            {
                std::lock_guard<std::mutex> lock(metrics_mtx_);
                metrics_.total_captured++;
                metrics_.queue_size = frame_queue_.size();
                metrics_.dropped_frames = local_dropped_count_; // 使用本地累计值
            }

            // FPS 计算（滑动窗口法计算）
            frames_in_interval_++;
            auto now = std::chrono::steady_clock::now();
            auto elapsed_sec = std::chrono::duration<double>(now - last_fps_update_).count();

            if (elapsed_sec >= FPS_INTERVAL_SEC) {
                double current_fps = static_cast<double>(frames_in_interval_) / elapsed_sec;
                {
                    std::lock_guard<std::mutex> lock(metrics_mtx_);
                    metrics_.current_fps = current_fps; // 更新FPS
                }
                // 重置计数器
                frames_in_interval_ = 0;
                last_fps_update_ = now;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "[ERROR] Camera " << camera_id_ << " capture error: " << e.what() << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(CAPTURE_ERROR_DELAY_MS));
        }
        catch (...) {
            std::cerr << "[ERROR] Camera " << camera_id_ << " unknown capture error!" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(CAPTURE_ERROR_DELAY_MS));
        }
    }
}

/*************************************************************************
* 改动历史纪录：
Revision 1.0, 2026-01-20, lium
describe: 初始创建.
*************************************************************************/