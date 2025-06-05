/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: objectdetection.h
*   软件模块: 目标检测
*   版 本 号: 1.0
*   生成日期: 2025-05-29
*   作    者: lium
*   功    能: 图像检测头文件声明
* 
************************************************************************/

#ifndef OBJECTDETECTION_H
#define OBJECTDETECTION_H

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>


/**检测结果 */
typedef struct {
    cv::Rect box;                    // 检查框
    std::string probability;              // 类别概率   
    std::string className;           // 类别名称
} TDetectResult;

class CObjDetection {
public:
    explicit CObjDetection(const std::string &modelFile, std::string &classFile);
    ~CObjDetection();

public:
    /**目标检测任务 */
    std::vector<TDetectResult> imgDetectTask(std::string &imgPath);

private:
    typedef enum {
        CPU,
        GPU
    } TDeviceType;

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
    void deInit(void);

    /**读取类别标签 */
    void readClassLabel(void);

    /**读取图像 */
    cv::Mat readImg(const std::string &imgPath);

    /**数据预处理 */
    cv::Mat preprocess(cv::Mat &srcImg);

    /**模型推理 */
    std::vector<cv::Mat> modelInference(cv::Mat &tensor4D, TDeviceType deviceType);

    /**后处理 */
    std::vector<TDetectResult> postprocess(std::vector<cv::Mat> &modelOutResult, cv::Mat &srcImg);

    /**在图像上绘制标签 */
    void drawLabel(cv::Mat &srcImg, const std::string &drawText, int x_startPoint, int y_startPoint);
private:
    /**类别名称 */
    std::vector<std::string> className_;

    /**模型文件 */
    const std::string modelFile_;

    /**类别文件 */
    std::string classFile_;

    /**模型需要的尺寸 */
    static const float INPUT_WIDTH;
    static const float INPUT_HEIGHT;

    /**阀值 */
    static const float SCORE_THRESHOLD;
    static const float NMS_THRESHOLD;
    static const float CONFIDENCE_THRESHOLD;
};

#endif /**OBJECTDETECTION_H */

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-05-29, lium
* describe: 初始创建.
*************************************************************************/