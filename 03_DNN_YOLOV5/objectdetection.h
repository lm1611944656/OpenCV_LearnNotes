#ifndef __OBJECTDETECTION_H__
#define __OBJECTDETECTION_H__

#include <iostream>
#include <map>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

using namespace cv;
using namespace cv::dnn;


/** 保存数据 */
typedef struct {
    std::vector<float> classProbabilitieOfMax;      // 将所有的类别概率最大值保存
    std::vector<cv::Rect> ROIs;                     // 保存置信度和类别都符合要求的ROI区域
    std::vector<int> maxClassProbabilitieOfIndex;   // 保存最大类别概率所在索引
}TDetectionResult;

class ObjectDetection{
public:
    explicit ObjectDetection(const std::string &modelPath, const std::string &classLabel, const std::string &imgPath);
    ~ObjectDetection();

    void modelInfer(void);

private:
    /**读取图像 */
    cv::Mat readImg(const std::string &imgPath);

    /**获取图像的宽高 */
    std::map<std::string, int> getImgWidthAndHeight(cv::Mat &srcImg);

    /**将图像转换为一个4维张量 [N, C, H, W] */
    cv::Mat imgToTensor(cv::Mat &srcImg);

    /** 读取到的图像进行预处理，返回预处理结果 */
    cv::Mat imgPreprocess(cv::Mat &srcImg);

    /** 输入图像的缩放因子 */
    std::vector<float> imgScalingFactor(cv::Mat &srcImg);

    /**获取标签信息 */
    std::vector<std::string> getClassNames(void);

    /** 获取模型 */
    cv::dnn::Net getNetModel(void);

    /** 设置模型的推理设备 */
    void setModelDevices(cv::dnn::Net &model);

    /** 模型的前向传播 */
    cv::Mat modelForward(cv::dnn::Net &model, cv::Mat &tensor);

    /** 模型输出后处理 */
    void modelOutputProcess(cv::Mat &outTensor);

    /** 非极大值抑制处理 */
    void nonMaxSuppressionProcess(cv::Mat &srcImg, std::vector<std::string> &class_names);

private:
    const std::string _model;
    const std::string _label;
    const std::string _imgPath;

    /** 输入模型的图像高度和宽度 */
    static double modelNeedImgWidth;  // 声明静态成员变量
    static double modelNeedImgHeight; // 壺明静态成员变量

    /** 设置置信度过滤阈值*/
    static const float confidence_threshold; // 全局置信度阈值

    /** 图像的缩放因子 */
    std::vector<float> scalingFactor;

    /** 检测结果 */
    TDetectionResult _detectionResult;

    int64 startTime;
};


#endif /**__OBJECTDETECTION_H__ */

