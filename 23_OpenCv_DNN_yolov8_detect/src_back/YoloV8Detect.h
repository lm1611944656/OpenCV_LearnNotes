#ifndef YOLOV8DETECT_H
#define YOLOV8DETECT_H

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <vector>
#include <string>

struct OutputSeg {
    int id;                 // 类别ID
    float confidence;       // 置信度
    cv::Rect box;           // 检测框位置和大小
    cv::Mat boxMask;        // 可选：如果存在分割信息，则存储对应的mask
};


class YoloV8Detect {
public:
    YoloV8Detect();
    bool detect(cv::Mat& cv_src, std::vector<OutputSeg>& output);
    void image_detect(cv::Mat &cv_src, std::vector<cv::Scalar> color);
    void video_detect(cv::VideoCapture &cap, std::vector<cv::Scalar> color);

    // 加载模型和类别名的方法
    void loadClassNames(std::string classNamesPath);
    void loadModel(const std::string& modelPath);

private:
    int _net_width = 640; // 默认网络输入宽度
    int _net_height = 640; // 默认网络输入高度
    float _class_threshold = 0.5f; // 类别置信度阈值
    float _nms_threshold = 0.4f; // NMS 阈值
    std::vector<std::string> _class_name; // 类别名称列表

    cv::dnn::Net _net; // 深度学习网络对象
};

#endif // YOLOV8DETECT_H