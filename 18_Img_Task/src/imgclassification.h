/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: imgclassification.h
*   软件模块: 图像分类
*   版 本 号: 1.0
*   生成日期: 2025-05-29
*   作    者: lium
*   功    能: 图像分类头文件声明
* 
************************************************************************/

#ifndef IMGCLASSIFICATION_H
#define IMGCLASSIFICATION_H

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

/**分类结果 */
typedef struct {
    std::string className;                  // 模型预测的类别名称
    double probability;                     // 模型预测的类别概率   
    std::vector<float> allProbability;      // 所有类别的预测概率
} TClassificationResult;

class CImgClassification {
public:
    explicit CImgClassification(const std::string &modelFile, std::string &classFile);
    ~CImgClassification();

public:
    /**图像分类任务 */
    TClassificationResult imgClsTask(std::string &imgPath);

private:
    /** 数据结构初始化 */
    void deInit(void);

    /** softmax函数 */
    cv::Mat softmax(const cv::Mat& logits);

    /** 加载类别标签 */
    void loadClassLabel(void);

    /**读取图像 */
    cv::Mat readImg(std::string &imgPath);

    /**预处理输入数据 */
    cv::Mat preprocess(cv::Mat &srcImg);

    /**模型推理 */
    cv::Mat modelInference(cv::Mat &tensor4D, bool isUsedCUDA = false);

    /**后处理输出数据 */
    //void postprocess(cv::Mat &tensor);

    /**后处理输出数据 */
    TClassificationResult postprocess(cv::Mat &tensor);

private:
    /**模型文件 */
    const std::string modelFile_;

    /**类别名 */
    std::string classFile_;

    /** 模型输入的尺寸 */
    static const int width_;
    static const int height_;

    /** 类别容器 */
    std::vector<std::string> classLabel;
};

#endif /**IMGCLASSIFICATION_H */

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-05-29, lium
* describe: 初始创建.
*************************************************************************/