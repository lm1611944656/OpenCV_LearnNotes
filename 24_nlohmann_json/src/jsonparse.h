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

#ifndef JOSNPARSE_H
#define JOSNPARSE_H

#include <iostream>
#include <fstream>
#include <string>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

#define MAX_CAMERAS         50  // 假设最多有50个摄像头
#define MAX_STRING_LEN      100 // 假设每个字符串的最大长度为100

/**单个摄像头的基本信息 */
typedef struct 
{
    char cameraIPAddr[20];
    char cameraPort[5];
    char cameraUserName[10];
    char cameraPasswd[10];
} TCameraBaseInfo_t;

typedef struct
{
    int camerasNums;                                // 实际摄像头数量
    TCameraBaseInfo_t camerasBaseInfo[MAX_CAMERAS]; // 摄像头信息数组
}TCamerasInfo_t;

class CJsonParse
{
public:
    explicit CJsonParse(const std::string &jsonPath);
    ~CJsonParse() = default;

     TCamerasInfo_t getCamerasInfo();
private:
    void initJsonInfo();

private:
    /**存储解析后的 JSON 数据 */
    json m_jsonDoc; 

    /**存储json文件 */
    std::string jsonFilePath_;
};

#endif /**JOSNPARSE_H */

/*************************************************************************
 * 改动历史纪录：
 * Revision 1.0, 2025-07-16, lium
 * describe: 初始创建.
 *************************************************************************/