#ifndef IMAGECOORDINATES_H
#define IMAGECOORDINATES_H

#include <opencv2/opencv.hpp>
#include <string>

class CImgCoordinates {
public:
    explicit CImgCoordinates(const std::string &imgPath);
    ~CImgCoordinates();

    /** 获取图像点击坐标 */
    static void getImgCoordinates(int event, int x, int y, int flags, void* userdata);

    const cv::Mat& getImage() const;
private:
    /** 读取图像 */
    void imRead(const std::string &imgPath);

private:
    cv::Mat srcImg;
    const std::string _imgPath;
};

#endif // IMAGECOORDINATES_H