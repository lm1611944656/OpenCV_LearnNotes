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
    cv::Rect box;                           // 检查框
    std::string className;
    std::string probability; 
} TDetectResult;


class CObjDetection {
public:

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

private:
    /**读取类别标签 */
    void readClassLabel(void);

    /**读取图像 */
    void readImg(const std::string &imgPath);

    /**数据预处理 */
    cv::Mat preprocess(void);

    /**模型推理 */
    cv::Mat modelInference(cv::Mat &tensor4D, TDeviceType deviceType);

    /**后处理 */
    void postprocess(cv::Mat &preds, cv::Mat &srcImg, std::vector<TDetectResult> *result);

private:
    /**类别名称 */
    std::vector<std::string> className_;

    /**模型文件 */
    const std::string modelFile_;

    /**类别文件 */
    std::string classFile_;

    /**缩放因子 */
    float x_factor;
    float y_factor;

    /**原始图像 */
    cv::Mat srcImg_;
    int64 startTick_;

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