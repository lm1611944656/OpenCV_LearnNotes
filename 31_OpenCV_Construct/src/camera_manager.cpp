/************************************************************************
*
*   Copyright (C), 2012-2020, GTBS. Co., Ltd.
*
*   软件模块 ：基础框架
*   功   能	：相机管理器(统一管理多相机)
*
*************************************************************************/
#include <iostream>
#include <map>
#include "camera_manager.h"
#include "base_camera.h"      

// 析构函数
CameraManager::~CameraManager() {
    stopAllCapture();
    closeAllCameras();
}

// 添加相机
bool CameraManager::addCamera(std::unique_ptr<BaseCamera> camera) 
{
    if (!camera) {
        std::cerr << "[ERROR] Null camera pointer!" << std::endl;
        return false;
    }

    const std::string& cam_id = camera->getCameraId();
    std::lock_guard<std::mutex> lock(mtx_);
    if (cameras_.find(cam_id) != cameras_.end()) {
        std::cerr << "[WARNING] Camera " << cam_id << " already exists!" << std::endl;
        return false;
    }

    cameras_[cam_id] = std::move(camera);
    std::cout << "[INFO] Camera " << cam_id << " added to manager!" << std::endl;
    return true;
}

// 初始化所有相机
bool CameraManager::initAllCameras()
{
    std::lock_guard<std::mutex> lock(mtx_);
    size_t success_count = 0;
    for (std::map<std::string, std::unique_ptr<BaseCamera> >::const_iterator it = cameras_.begin();
         it != cameras_.end(); ++it) {
        if (it->second && it->second->initCamera()) {
            ++success_count;
        }
    }

    bool all_success = (success_count == cameras_.size());
    std::cout << "[INFO] Camera init finished: "
              << success_count << "/" << cameras_.size() << " success!" << std::endl;
    return all_success;
}

// 启动所有采集
bool CameraManager::startAllCapture() {
    std::lock_guard<std::mutex> lock(mtx_);
    size_t success_count = 0;
    for (std::map<std::string, std::unique_ptr<BaseCamera> >::iterator it = cameras_.begin();
         it != cameras_.end(); ++it) {
        std::unique_ptr<BaseCamera>& camera = it->second;
        if (camera && camera->startCapture()) {
            ++success_count;
        }
    }

    bool all_success = (success_count == cameras_.size());
    std::cout << "[INFO] Capture start finished: "
              << success_count << "/" << cameras_.size() << " success!" << std::endl;
    return all_success;
}

// 停止所有采集
void CameraManager::stopAllCapture() {
    std::lock_guard<std::mutex> lock(mtx_);
    for (auto& kv : cameras_) {
        //const std::string& cam_id = kv.first;
        std::unique_ptr<BaseCamera>& camera = kv.second;
        if (camera) {
            camera->stopCapture();
        }
    }

    std::cout << "[INFO] All cameras capture stopped!" << std::endl;
}

// 关闭所有相机
void CameraManager::closeAllCameras() {
    std::lock_guard<std::mutex> lock(mtx_);
    for (auto& kv : cameras_) {
        //const std::string& cam_id = kv.first;
        std::unique_ptr<BaseCamera>& camera = kv.second;
        if (camera) {
            camera->closeCamera();
        }
    }

    cameras_.clear();
    std::cout << "[INFO] All cameras closed and released!" << std::endl;
}

// 获取指定相机
BaseCamera* CameraManager::getCamera(const std::string& camera_id) 
{
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = cameras_.find(camera_id);
    return (it != cameras_.end()) ? it->second.get() : nullptr;
}

// 获取所有最新帧
std::map<std::string, FrameData> CameraManager::getAllFrames(int timeout_ms) {
    std::map<std::string, FrameData> all_frames;
    std::lock_guard<std::mutex> lock(mtx_);
    for (auto& kv : cameras_) {
        const std::string& cam_id = kv.first;
        std::unique_ptr<BaseCamera>& camera = kv.second;
        if (camera) {
            FrameData frame = camera->getFrame(timeout_ms);
            all_frames[cam_id] = std::move(frame);
        }
    }
    
    return all_frames;
}

// 清空所有队列
void CameraManager::clearAllQueues() {
    std::lock_guard<std::mutex> lock(mtx_);
    for (auto& kv : cameras_) {
        //const std::string& cam_id = kv.first;
        std::unique_ptr<BaseCamera>& camera = kv.second;
        if (camera) {
            camera->clearQueue();
        }
    }
}

/*************************************************************************
* 改动历史纪录：
Revision 1.0, 2026-01-20, lium
describe: 初始创建.
*************************************************************************/