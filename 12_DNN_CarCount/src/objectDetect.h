/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: objectDetect.h
*   软件模块: 应用模块
*   版 本 号: 1.0
*   生成日期: 2025-04-17
*   作    者: lium
*   功    能: 车辆计数(目标检测)
* 
*************************************************************************/

#ifndef __OBJECTDETECT_H__
#define __OBJECTDETECT_H__

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

typedef enum {
    CPU,
    GPU
}TDeviceType;

class CObjeectDetect{
public:
    explicit CObjeectDetect(const std::string &modelPath, const std::string &labelPath);
    ~CObjeectDetect();

    /**图像推理 */
    void imageInference(const std::string &imgPath);

    /**视频推理 */
    void videoInference(const std::string &videoPath);

    /*********************************************************** */
    const int IMAGE_WIDTH = 1280;
    const int IMAGE_HEIGHT = 720;
    const int LINE_HEIGHT = IMAGE_HEIGHT / 2;

    /**车辆计数 */
    void vehicleCounting(const std::string &videoPath);

    /**计算矩形的中心点坐标点 */
    std::vector<cv::Point> getCenterCoordinatesOfRectangle(std::vector<cv::Rect> &detections);
    
    /**计算两点之间的距离 */
    float euclideanDistance(cv::Point &startPoint, cv::Point &endPoint);

    /**判断两点是否跨越某一条直线 */
    bool is_cross(cv::Point p1, cv::Point p2);
    /********************************************************** */

private:
    /**颜色 */
    typedef enum{
        COLOR_RED,
        COLOR_GREEN,
        COLOR_BLUE,
        COLOR_YELLOW,
        COLOR_CYAN,
        COLOR_MAGENTA,
        COLOR_BLACK,
        COLOR_WHITE,
        COLOR_GRAY,
        COLOR_ORANGE,
        COLOR_PURPLE,
        COLOR_BROWN,

        COLOR_NUM       // 颜色的数量
    }TSelfColorType;

    const std::map<TSelfColorType, cv::Scalar> color = {
        {COLOR_RED,     cv::Scalar(0, 0, 255)},       // BGR for Red
        {COLOR_GREEN,   cv::Scalar(0, 255, 0)},       // BGR for Green
        {COLOR_BLUE,    cv::Scalar(255, 0, 0)},       // BGR for Blue
        {COLOR_YELLOW,  cv::Scalar(0, 255, 255)},     // BGR for Yellow
        {COLOR_CYAN,    cv::Scalar(255, 255, 0)},     // BGR for Cyan
        {COLOR_MAGENTA, cv::Scalar(255, 0, 255)},     // BGR for Magenta
        {COLOR_BLACK,   cv::Scalar(0, 0, 0)},         // BGR for Black
        {COLOR_WHITE,   cv::Scalar(255, 255, 255)},   // BGR for White
        {COLOR_GRAY,    cv::Scalar(128, 128, 128)},   // BGR for Gray
        {COLOR_ORANGE,  cv::Scalar(0, 165, 255)},     // BGR for Orange
        {COLOR_PURPLE,  cv::Scalar(128, 0, 128)},     // BGR for Purple
        {COLOR_BROWN,   cv::Scalar(42, 42, 165)}      // BGR for Brown
    };
private:
    void deinit(void);

    /**读取类别标签 */
    void readClassLabel(void);

    /**读取图像 */
    cv::Mat readImg(const std::string &imgPath);

    /**数据预处理 */

    /**前处理 */
    cv::Mat pre_process(cv::Mat &srcImg);

    /**模型推理 */
    std::vector<cv::Mat> modelInference(cv::Mat &tensor4D, TDeviceType deviceType);

    /**后处理 */
    std::vector<cv::Rect> post_process(std::vector<cv::Mat> &modelOutResult, cv::Mat &srcImg);
    std::vector<cv::Rect> post_process(std::vector<cv::Mat> &modelOutResult, cv::Mat &srcImg, const std::string &className);

    /**绘制图像 */
    void plotImg(cv::Mat &srcImg, const char *winName);

    /**在图像上绘制标签 */
    void drawLabel(cv::Mat &srcImg, const std::string &drawText, int x_startPoint, int y_startPoint);

    /**获取Mat信息 */
    void getMatInf(cv::Mat &_mat);

    /**获取模型信息 */
    void getModelInf(cv::dnn::Net &model);


private:
    const std::string _modelPath;
    const std::string _classLabelPath;

    /**输入图像的宽和高 */
    static const float INPUT_IMG_WIDTH;
    static const float INPUT_IMG_HEIGHT;
    static const float SCORE_THRESHOLD;             // 类别得分阈值，用于过滤低置信度预测
    static const float NMS_THRESHOLD;               // 非极大值抑制（NMS）的交并比（IoU）阈值
    static const float CONFIDENCE_THRESHOLD;        // 置信度阈值，用于过滤低置信度框

    /**存储类别标签 */
    std::vector<std::string> _className;
};

#endif 


/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-04-17, lium
* describe: 初始创建.
*************************************************************************/

