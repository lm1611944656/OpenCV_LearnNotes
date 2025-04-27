#include "networkInterfaceDetect.h"
#include <opencv2/photo.hpp>


CNetworkInterfaceDetect::CNetworkInterfaceDetect(const std::string &imgPath)
:_imgPath(imgPath){
    deInit();
    initForm();

    /**读取图像 */
    cv::Mat srcImg = readImg();
    plotImg(srcImg, "srcImg");

    /**将图像转换到HSV空间，做颜色检测 */
    cv::Mat binaryImg = imgBGR2HSV(srcImg);

    /**图像形态学操作 */
    cv::Mat dstBinaryImg = _dilate(binaryImg);

    /**图像修复 */
    cv::Mat dstImg = imgInpainting(srcImg, dstBinaryImg);
    plotImg(dstImg, "dstImg");
}

CNetworkInterfaceDetect::~CNetworkInterfaceDetect(void){

}

void CNetworkInterfaceDetect::deInit(void){

}

void CNetworkInterfaceDetect::initForm(void){

}

cv::Mat CNetworkInterfaceDetect::readImg(void){
    if(this->_imgPath.empty()){
        throw std::invalid_argument("传递参数错误！");
    }

    cv::Mat srcImg;
    srcImg = cv::imread(this->_imgPath, cv::IMREAD_COLOR);
    return srcImg;
}

cv::Mat CNetworkInterfaceDetect::imgBGR2HSV(cv::Mat &srcImg){
    if(srcImg.empty()){
        throw std::invalid_argument("传递参数错误！");
    }

    /**空间变化 */
    cv::Mat hsvImg;
    cv::cvtColor(srcImg, hsvImg, cv::COLOR_BGR2HSV);

    cv::namedWindow("hsvImg", cv::WINDOW_AUTOSIZE);
    cv::imshow("hsvImg", hsvImg);

    /**将范围以内的像素标记为白色,范围以外的生成黑色 */
    cv::Mat binaryImg;
    cv::inRange(srcImg, cv::Scalar(10, 50, 255), cv::Scalar(40, 255, 255), binaryImg);

    return binaryImg;
}

cv::Mat CNetworkInterfaceDetect::_dilate(cv::Mat &binaryImg){
    if(binaryImg.empty()){
        throw std::invalid_argument("传递参数错误！");
    }
    
    cv::Mat dstBinaryImg;

    /**创建一个卷积核 */
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size());
    cv::dilate(binaryImg, dstBinaryImg, kernel);
    return dstBinaryImg;
}



cv::Mat CNetworkInterfaceDetect::imgInpainting(cv::Mat &srcImg, cv::Mat &binaryImg){
    if(srcImg.empty() && binaryImg.empty()){
        throw std::invalid_argument("传递参数错误！");
    }

    cv::Mat inpaintImg;
    inpaint(srcImg, binaryImg, inpaintImg, 3, cv::INPAINT_NS);

    return inpaintImg;
}


void CNetworkInterfaceDetect::plotImg(cv::Mat &srcImg, const std::string &winName){
    if(srcImg.empty()){
        throw std::invalid_argument("传递参数错误！");
    }
    cv::namedWindow(winName, cv::WINDOW_AUTOSIZE);
    cv::imshow(winName, srcImg);
}