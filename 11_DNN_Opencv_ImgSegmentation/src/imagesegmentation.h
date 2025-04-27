/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: imagesegmentation.h
*   软件模块: 应用模块
*   版 本 号: 1.0
*   生成日期: 2025-04-16
*   作    者: lium
*   功    能: 图像分割
* 
*************************************************************************/

#ifndef __IMAGESEGMENTATION_H__
#define __IMAGESEGMENTATION_H__

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

class ImgSegmentation{
public:
    explicit ImgSegmentation(const std::string &modelPath, const std::string &labelPath, const std::string &imgPath);
    ~ImgSegmentation();

    /**图像推理检测 */
    void imageInferenceDetection(void);

    /**视频推理检测 */
    void videoInferenceDetection(const std::string &videoPath);

private:
    /**掩膜相关参数 */
    typedef struct{
        int segChannels = 32;   // 分割通道数（掩膜特征维度）
        int segWidth = 160;     // 分割掩膜的宽度
        int segHeight = 160;    // 分割掩膜的高度
        int netWidth = 640;     // 网络输入宽度
        int netHeight = 640;    // 网络输入高度
        float maskThreshold = 0.25; // 掩膜二值化阈值
        cv::Size srcImgShape;   // 原始输入图像的尺寸
        cv::Vec4d params;       // LetterBox处理中的参数 [ratio_x, ratio_y, dw, dh]
    }TMaskParams;

    /**网络输出相关参数 */
    typedef struct {
        int id;             //结果类别id
        float confidence;   //结果置信度
        cv::Rect box;       //矩形框
        cv::Mat boxMask;    //矩形框内mask，节省内存空间和加快速度
    }TOutputSeg;

private:
    void deinit(void);

    /**读取标签 */
    void readLabel(void);

    /**读取图像 */
    cv::Mat readImg(const std::string &imgPath);

    /**数据预处理 */
    void imageBorderPadding(
        const cv::Mat& srcImg,          // 输入参数：原始图像
        cv::Mat& dstImg,                // 输出参数：变化后的图像   
        cv::Vec4d& params,              // 输出参数：[ratio_x, ratio_y, dw, dh]
        const cv::Size& newShape = cv::Size(640, 640),
        bool autoShape = false,         // 自动填充
        bool scaleFill = false,         // 强制填充
        bool scaleUp = true,
        int stride = 32,
        const cv::Scalar& color = cv::Scalar(114, 114, 114));
    cv::Mat pre_process(cv::Mat &srcImg, cv::Vec4d &paddingParam);

    /**模型推理 */
    void modelInference(cv::Mat &tensor4D, bool isUseCUDA);

    /**根据模型输出，生成分掩膜 */
    void generateMask(
        const cv::Mat& maskProposals,       // 当前检测框对应的物体大致轮廓
        const cv::Mat& mask_protos,         // 基础掩膜图案大全
        TOutputSeg& output,                 // 语义分割到的类别信息
        const TMaskParams& maskParams);     // 掩膜相关参数，帮助函数正确地把轮廓映射回原始图像

    /**后处理：解析网络输出，生成最终结果 */
    cv::Mat post_process(cv::Mat& image, std::vector<cv::Mat>& outputs, const std::vector<std::string>& class_name, cv::Vec4d& params);

    /**在原始图像上绘制检测框和掩膜 */
    void plotResult(cv::Mat &srcImg, std::vector<TOutputSeg> result, std::vector<std::string> class_name);

private:
    /**模型、类别标签和图像路径 */
    std::string _modelPath;
    std::string _labelPath;
    std::string _imgPath;

    /**类别标签 */
    std::vector<std::string> classLabel;

    /**模型输出tensor */
    std::vector<cv::Mat> modelOutTensor;

    /**静态常量定义 */
    static const int INPUT_IMG_WIDTH;               // 输入图像宽度
    static const int INPUT_IMG_HEIGHT;              // 输入图像高度
    static const float SCORE_THRESHOLD;             // 类别得分阈值，用于过滤低置信度预测
    static const float NMS_THRESHOLD;               // 非极大值抑制（NMS）的交并比（IoU）阈值
    static const float CONFIDENCE_THRESHOLD;        // 置信度阈值，用于过滤低置信度框
};


#endif

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-04-16, lium
* describe: 初始创建.
*************************************************************************/