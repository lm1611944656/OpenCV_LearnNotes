/*************************************************************************
 *
 *   Copyright (C), 2017-2027, BPG. Co., Ltd.
 *
 *   文件名称: jsonparam.h
 *   软件模块: 目标检测
 *   版 本 号: 1.0
 *   生成日期: 2025-07-18
 *   作    者: lium
 *   功    能: json对象解析
 *
 ************************************************************************/
#include "jsonparse.h"
#include <sstream>

CJsonParse::CJsonParse(const std::string &jsonPath) 
:jsonFilePath_(jsonPath)
{
    initJsonInfo();
}

void CJsonParse::initJsonInfo()
{
    std::ifstream file(jsonFilePath_);
    if (!file.is_open()) {
        std::cerr << "无法打开 JSON 文件: " << jsonFilePath_ << std::endl;
        return;
    }

    try {
        file >> m_jsonDoc;
    } catch (const json::parse_error& e) {
        std::cerr << "JSON 解析失败: " << e.what() << std::endl;
    }
}

TCamerasInfo_t CJsonParse::getCamerasInfo() {
    TCamerasInfo_t info = {
        .camerasNums = 0,
        .camerasBaseInfo = { {0} } // 这里初始化第一个cameraBaseInfo，其他会自动填充为0
    };
     
    // 检查 JSON 对象中是否存在 "cameras" 字段
    if (m_jsonDoc.contains("cameras")) {
        const json &cameras = m_jsonDoc["cameras"];

        // 遍历 cameras 对象
        for (auto it = cameras.begin(); it != cameras.end(); ++it) {
            std::string name = it.key();     // 获取摄像头名称（如 "camera0"）
            const json &camera = it.value(); // 获取摄像头信息对象

            // 只处理 enable == true 的摄像头
            if (camera.value("enable", false)) {
                if (info.camerasNums >= MAX_CAMERAS) {
                    // 超出最大支持数量，防止数组越界
                    break;
                }

                // 获取字段值并拷贝到结构体中
                std::string ip = camera.value("ip", "");
                std::string port = camera.value("port", "");
                std::string username = camera.value("username", "");
                std::string password = camera.value("password", "");

                // 拷贝字符串并确保结尾有 '\0'
                memset(info.camerasBaseInfo[info.camerasNums].cameraIPAddr, '\0', sizeof(info.camerasBaseInfo[info.camerasNums].cameraIPAddr));
                strncpy(info.camerasBaseInfo[info.camerasNums].cameraIPAddr, ip.c_str(), sizeof(info.camerasBaseInfo[info.camerasNums].cameraIPAddr) - 1);
                info.camerasBaseInfo[info.camerasNums].cameraIPAddr[sizeof(info.camerasBaseInfo[info.camerasNums].cameraIPAddr) - 1] = '\0';

                memset(info.camerasBaseInfo[info.camerasNums].cameraPort, '\0', sizeof(info.camerasBaseInfo[info.camerasNums].cameraPort));
                strncpy(info.camerasBaseInfo[info.camerasNums].cameraPort, port.c_str(), sizeof(info.camerasBaseInfo[info.camerasNums].cameraPort) - 1);
                info.camerasBaseInfo[info.camerasNums].cameraPort[sizeof(info.camerasBaseInfo[info.camerasNums].cameraPort) - 1] = '\0';

                memset(info.camerasBaseInfo[info.camerasNums].cameraUserName, '\0', sizeof(info.camerasBaseInfo[info.camerasNums].cameraUserName));
                strncpy(info.camerasBaseInfo[info.camerasNums].cameraUserName, username.c_str(), sizeof(info.camerasBaseInfo[info.camerasNums].cameraUserName) - 1);
                info.camerasBaseInfo[info.camerasNums].cameraUserName[sizeof(info.camerasBaseInfo[info.camerasNums].cameraUserName) - 1] = '\0';

                memset(info.camerasBaseInfo[info.camerasNums].cameraPasswd, '\0', sizeof(info.camerasBaseInfo[info.camerasNums].cameraPasswd));
                strncpy(info.camerasBaseInfo[info.camerasNums].cameraPasswd, password.c_str(), sizeof(info.camerasBaseInfo[info.camerasNums].cameraPasswd) - 1);
                info.camerasBaseInfo[info.camerasNums].cameraPasswd[sizeof(info.camerasBaseInfo[info.camerasNums].cameraPasswd) - 1] = '\0';

                info.camerasNums++; // 摄像头计数增加
            }
        }
    }

    return info;
}

/*************************************************************************
 * 改动历史纪录：
 * Revision 1.0, 2025-07-16, lium
 * describe: 初始创建.
 *************************************************************************/