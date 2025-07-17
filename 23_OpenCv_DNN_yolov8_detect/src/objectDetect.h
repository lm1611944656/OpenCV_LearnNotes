/*************************************************************************
 *
 *   Copyright (C), 2017-2027, BPG. Co., Ltd.
 *
 *   文件名称: objectdetect.h
 *   软件模块: 目标检测
 *   版 本 号: 1.0
 *   生成日期: 2025-07-17
 *   作    者: lium
 *   功    能: 图像检测头文件声明
 *
 ************************************************************************/

#ifndef OBJECTDETECT_H
#define OBJECTDETECT_H

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <fstream>

typedef struct
{
    int id;                // 类别ID
    float confidence;      // 类别概率
    std::string className; // 类别名称
    cv::Rect box;          // anchor box
    cv::Mat boxMask;       // segmentation mask(可选)
} TDetectResult_t;

class CObjectDetect
{
public:
    explicit CObjectDetect(const std::string &modelPath, const std::string &classFilePath);
    ~CObjectDetect() = default;

    /**模型推理 */
    void modelDetect(const std::string &srcImg);

private:
    typedef enum
    {
        CPU,
        GPU
    } TDeviceType_t;

private:
    /**letterBox */
    void letterBox(const cv::Mat &image, cv::Mat &outImage,
                   cv::Vec4d &params, //[ratio_x,ratio_y,dw,dh]
                   const cv::Size &newShape = cv::Size(640, 640),
                   bool autoShape = false,
                   bool scaleFill = false,
                   bool scaleUp = true,
                   int stride = 32,
                   const cv::Scalar &color = cv::Scalar(114, 114, 114));

    /**box缩放到原图尺寸 */
    void scaleBoxes(cv::Rect &box, cv::Size size);

    /**
	 * @description: 						Non-Maximum Suppression
	 * @param {vector<cv::Rect>} & boxes	detect bounding boxes
	 * @param {vector<float>} &	scores		detect scores		
	 * @param {float} score_threshold		detect score threshold
	 * @param {float} nms_threshold			IOU threshold
	 * @param {vector<int>} & indices		detect indices
	 * @return {*}
	 */
	void nms(std::vector<cv::Rect> & boxes, std::vector<float> & scores, float score_threshold, float nms_threshold, std::vector<int> & indices);

    /**读取类别标签 */
    void readClassLabel(void);

    /**读取图像 */
    void readImg(const std::string &srcImgPath);

    /**数据预处理 */
    cv::Mat preprocess(void);

    /**模型推理 */
    std::vector<cv::Mat> modelInference(cv::Mat &blob, TDeviceType_t deviceType);

    /**后处理 */
    std::vector<TDetectResult_t> postprocess(std::vector<cv::Mat> &netOut);

    // 可视化函数
    void drawResult(cv::Mat &srcImg, std::string label, cv::Rect box);

private:
    const std::string modelPath_;
    const std::string classFilePath_;

    /**存储类别名称 */
    std::vector<std::string> className_;

    /**原始图像 */
    cv::Mat srcImg_;
};

#endif /**OBJECTDETECT_H */

/*************************************************************************
 * 改动历史纪录：
 * Revision 1.0, 2025-07-16, lium
 * describe: 初始创建.
 *************************************************************************/