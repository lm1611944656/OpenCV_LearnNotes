#ifndef __IMAGECLASSIFICATION_H__
#define __IMAGECLASSIFICATION_H__

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

class ImageClassification{
public:
    explicit ImageClassification(const std::string &modelFile, const std::string &classFiles);
    ~ImageClassification();

public:
    /** 数据结构初始化 */
    void deInit(void);

    /** softmax函数 */
    cv::Mat softmax(const cv::Mat& logits);

    /** 加载类别标签 */
    void loadClassLabel(void);

    /**读取图像 */
    cv::Mat readImg(const std::string &imgPath);

    /**预处理输入数据 */
    cv::Mat preprocess(cv::Mat &srcImg);

    /**模型推理 */
    cv::Mat modelInference(cv::Mat &tensor4D, bool isUsedCUDA = false);

    /**后处理输出数据 */
    void postprocess(cv::Mat &tensor);

private:
    /** 模型和标签文件 */
    std::string m_modelFile;
    std::string m_classFile;

    /** 类别容器 */
    std::vector<std::string> classLabel;

    /** 模型输入的尺寸 */
    const static int width = 224;
    const static int height = 224;
};

#endif /**__IMAGECLASSIFICATION_H__ */
