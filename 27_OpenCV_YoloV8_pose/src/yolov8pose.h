/*************************************************************************
 *
 *   Copyright (C), 2017-2027, BPG. Co., Ltd.
 *
 *   文件名称: yolov8pose.h
 *   软件模块: 目标检测
 *   版 本 号: 1.0
 *   生成日期: 2025-07-23
 *   作    者: lium
 *   功    能: 关键点检测头文件声明
 *
 ************************************************************************/

#ifndef YOLOV8POSE_H
#define YOLOV8POSE_H

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <fstream>

/**关键点检测结果 */
typedef struct
{
    cv::Rect_<float> box;
    float confidence;
    std::vector<float> kps;
} TOutputPose_t;

class CYoloV8Pose
{
public:
    explicit CYoloV8Pose(const std::string &modelPath, const std::string &classNamePath);
    ~CYoloV8Pose();

    /**模型推理 */
    void modelDetect(const std::string &imgPath,
                     const std::vector<std::vector<unsigned int>> &skeleton,
                     const std::vector<std::vector<unsigned int>> &kps_colors,
                     const std::vector<std::vector<unsigned int>> &limb_colors);

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
    void scaleBoxes(cv::Rect &box);

    /**读取类别标签 */
    void readClassLabel(void);

    /**读取图像 */
    void readImg(const std::string &srcImgPath);

    /**数据预处理 */
    cv::Mat preprocess(void);

    /**模型推理 */
    std::vector<cv::Mat> modelInference(cv::Mat &blob, TDeviceType_t deviceType);

    /**后处理 */
    std::vector<TOutputPose_t> postprocess(std::vector<cv::Mat> &netOut);

    /**绘制检测框结果 */
    void drawResult(cv::Mat &srcImg, std::string label, cv::Rect box);

    /**绘制关键点和关键点之间的连线 */
    void drawKeyPoints(cv::Mat &img, const std::vector<TOutputPose_t> &keyPointsDetectResult,
                       const std::vector<std::vector<unsigned int>> &skeleton,
                       const std::vector<std::vector<unsigned int>> &kps_colors,
                       const std::vector<std::vector<unsigned int>> &limb_colors);

    /**根据关键点信息，做摔倒检测任务 */
    void fallDetectTask(cv::Mat &img, const std::vector<TOutputPose_t> &keyPointsDetectResult);

    /**绘制关键点，关键点之间的连接以及摔倒检测 */
    void drawPose(cv::Mat &img, const std::vector<TOutputPose_t> &keyPointsDetectResult,
                  const std::vector<std::vector<unsigned int>> &skeleton,
                  const std::vector<std::vector<unsigned int>> &kps_colors,
                  const std::vector<std::vector<unsigned int>> &limb_colors);
    
private:
    /**模型文件路径 */
    const std::string modelPath_;

    /**类别名称文件路径 */
    const std::string classFilePath_;

    /**存储类别名称 */
    std::vector<std::string> className_;

    /**原始图像 */
    cv::Mat srcImg_;

    /**[宽度方向的缩放比例, 高度方向的缩放比例, 左右两侧各填充量，上下两端各填充量] */
    cv::Vec4d params_;
};

#endif /**YOLOV8POSE_H */

/*************************************************************************
 * 改动历史纪录：
 * Revision 1.0, 2025-07-23, lium
 * describe: 初始创建.
 *************************************************************************/