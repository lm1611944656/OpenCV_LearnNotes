#ifndef IMGPROCESS_H
#define IMGPROCESS_H

#include <opencv2/opencv.hpp>

class CImgProcess {
public:
    explicit CImgProcess();
    ~CImgProcess();

public:
    void run(cv::Mat &srcImg, cv::Mat &resultImg);


private:
    void drawTextMultiLine(cv::Mat& img, const std::string& text, cv::Point org,
                       int fontFace, double fontScale, const cv::Scalar& color, int thickness);
    
};

#endif /**IMGPROCESS_H */