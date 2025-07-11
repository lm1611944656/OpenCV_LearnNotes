/*************************************************************************
 *
 *   Copyright (C), 2017-2027, BPG. Co., Ltd.
 *
 *   文件名称: TemperatureDetection.cpp
 *   软件模块: 应用模块
 *   版 本 号: 1.0
 *   生成日期: 2025-07-10
 *   作    者: lium
 *   功    能: 海康摄像头温度检测模块
 *
 *************************************************************************/

#ifndef TEMPERATUREDETECTION_H
#define TEMPERATUREDETECTION_H

#include <iostream>
#include <string>
#include <vector>
#include <algorithm> // std::min, std::max
#include <sstream>   // std::stringstream
#include <cmath>     // stof, stoi
#include <cstring>   // memcpy
#include <cstdint>   // int16_t, uint32_t
#include <curl/curl.h>
#include <fstream>
#include <numeric> //（用于 std::accumulate）
#include <map>
#include <float.h>

/**返回的温度数据对象 */
typedef struct{
    float temperaure;       // 温度值
    int x;                  // 温度值对应的x坐标点
    int y;                  // 温度值对应的y坐标点  
}TTemperaureData_t;

/**返回指定区域的温度数据值对象 */
typedef struct {
    float maxTemperaure;        // 指定区域的最高温
    float minTemperaure;        // 指定区域的最低温
    float averageTemperaure;    // 指定区域的平均温度
}TTemperatureStats_t;


/**用于存储响应数据及 Content-Type 类型 */
typedef struct  {
    std::vector<char> data;        // 响应主体数据
    std::string contentType;       // 头部中的 Content-Type 字段
}TWriteData_t;


class CTemperatureDetection
{
public:
    explicit CTemperatureDetection(const std::string &cameraInfo, const std::string &userName, const std::string &passwd);
    ~CTemperatureDetection();

    /**发生请求，并且解析响应 */
    void sendRequestAndParseData();

    /**获取区域的所有温度数据 */
    std::vector<float> getAreaAllTemperature();

    /**获取最高温度, 以及最高温的具体坐标点 */
    TTemperaureData_t getAreaMaxTemperature();

    /**获取最低温度，以及最高低温的具体坐标点 */
    TTemperaureData_t getAreaMinTemperature();

    /**获取平均温度 */
    float getAreaAverageTemperature();

    /**获取指定区域的温度数据值[(x, y)区域左上角坐标, (width, height)区域宽高] */
    TTemperatureStats_t calculateTemperatureStatsInRegion(int x_start, int y_start, int width, int height);

private:
    /**设置请求头 */
    void setRequestHeaderOfUrl();

    /**执行请求 */
    void executeRequest(CURL *curl);

    /**写入响应体的回调函数 */
    static size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata);
    
    /**写入头部信息的回调函数，用于提取 Content-Type 和 boundary */
    static size_t header_callback(void *ptr, size_t size, size_t nmemb, void *userdata);

    /**从 Content-Type 提取 boundary 字符串 */
    std::string extract_boundary(const std::string& contentType);

    /**将 multipart 数据按照 boundary 分割为多个部分 */
    std::vector<std::vector<char>> split_multipart(const std::vector<char>& data, const std::string& boundary);

    /**安全地将字符串转为 float */
    float parse_float(const std::string& s);

    /**安全地将字符串转为 int */
    int parse_int(const std::string& s);

    /**从 JSON 字符串中提取指定字段的值 */
    void extract_json_values(const std::string& json, int &width, int &height, int &data_len, float &scale, float &offset);

    /**解析温度数据（支持 2 字节或 4 字节格式） */
    std::vector<float> parse_temperature(const std::vector<char>& data, int width, int height, int data_len, float scale, float offset);

private:
    /**摄像头的URL */
    const std::string cameraInfo_;

    /**摄像头的用户名 */
    const std::string userName_;

    /**摄像头的秘密 */
    const std::string passwd_;

    /**请求对象 */
    CURL *curl;

    /**请求 */
    TWriteData_t responseData_;

    /**温度矩阵信息 */
    int width_;              /**温度矩阵的宽度 */
    int height_;             /**温度矩阵的高度 */
    int data_len_;           /**温度矩阵的深度(几个数据表示一个温度值) */
    float scale_;         
    float offset_;

    /**存储所有的温度数据 */
    std::vector<float> tempsDataArr_;
};

#endif /**TEMPERATUREDETECTION_H */

/*************************************************************************
 * 改动历史纪录：
 * Revision 1.0, 2025-07-10, lium
 * describe: 初始创建.
 *************************************************************************/