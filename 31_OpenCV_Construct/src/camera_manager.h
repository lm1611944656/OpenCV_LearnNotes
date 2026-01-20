/************************************************************************
*
*   Copyright (C), 2012-2020, GTBS. Co., Ltd.
*
*   软件模块 ：基础框架
*   功   能	：相机管理器(统一管理多相机)
*
*************************************************************************/
#ifndef __CAMERA_MANAGER_H__
#define __CAMERA_MANAGER_H__

#include <string>
#include <map>
#include <memory>
#include <mutex>
#include <iostream>

#include "config.h"

// 前置声明
class BaseCamera;
struct FrameData;

class CameraManager {
public:
    CameraManager() = default;
    ~CameraManager();

    // 禁止拷贝和移动
    CameraManager(const CameraManager&) = delete;
    CameraManager& operator=(const CameraManager&) = delete;
    CameraManager(CameraManager&&) = delete;
    CameraManager& operator=(CameraManager&&) = delete;

    // 添加相机（转移所有权）
    bool addCamera(std::unique_ptr<BaseCamera> camera);

    // 批量控制
    bool initAllCameras();
    bool startAllCapture();
    void stopAllCapture();
    void closeAllCameras();

    // 查询与操作
    BaseCamera* getCamera(const std::string& camera_id);
    std::map<std::string, FrameData> getAllFrames(int timeout_ms = QUEUE_TIMEOUT_MS);
    void clearAllQueues();

private:
    std::map<std::string, std::unique_ptr<BaseCamera>> cameras_;
    mutable std::mutex mtx_;
};

#endif /**__CAMERA_MANAGER_H__ */

/*************************************************************************
* 改动历史纪录：
Revision 1.0, 2026-01-20, lium
describe: 初始创建.
*************************************************************************/