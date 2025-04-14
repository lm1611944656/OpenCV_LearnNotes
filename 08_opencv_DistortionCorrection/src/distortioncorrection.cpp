/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: distortioncorrection.cpp
*   软件模块: 应用模块
*   版 本 号: 1.0
*   生成日期: 2025-04-10
*   作    者: lium
*   功    能: 图像畸变校正
* 
*************************************************************************/

#include "distortioncorrection.h"

CDistortionCorrection::CDistortionCorrection(const std::string &imgPath)
:_imgPath(imgPath){
    deInit();

    formInit();
}

CDistortionCorrection::~CDistortionCorrection(){

}

void CDistortionCorrection::deInit(void){
    /**图像 */
}

void CDistortionCorrection::formInit(void){
    /**读取图像 */
    cv::Mat srcImg = readImg();

    /**通道压缩 */
    cv::Mat grayImg = _cvtColor(srcImg);

    /**高斯滤波 */
    cv::Mat img = imgFilte(grayImg);

    /**图像二值化 */
    int value = _threshold(img, this->binaryImg, 0, 255);

    /**图像形态学操作 */
    cv::Mat binaryImg = morphologicalOperations(this->binaryImg);

#if 0
    cv::namedWindow("Binary Open Img", cv::WINDOW_AUTOSIZE);
    cv::imshow("Binary Open Img", binaryImg);
    cv::waitKey(20);
#endif
    /**查找轮廓 */
    std::vector<std::vector<cv::Point>> contours = _findContours(binaryImg);
    //plotCoutours(srcImg, contours); /**绘制图像 */

    /**轮廓逼近 */
    std::vector<cv::Point> srcPts = contoursApproach(contours);

    /**轮廓点排序 */
    std::map<std::string, cv::Point> contoursPoint = contoursPointsSort(srcImg, srcPts);
    // plotCoutours(srcImg, contoursPoint); /**绘制图像 */

    /**透视变化 */
    cv::Mat dstImg = imgCorrection(srcImg, contoursPoint);
    plotCoutours(dstImg);
}

cv::Mat CDistortionCorrection::getBinaryImg(void){
    return this->binaryImg;
}

cv::Mat CDistortionCorrection::readImg(void){
    cv::Mat srcImg = cv::imread(this->_imgPath, cv::IMREAD_COLOR);
    if (srcImg.empty()) {
        throw std::runtime_error("Failed to load image: " + this->_imgPath);
    }
    return srcImg;
}

cv::Mat CDistortionCorrection::_cvtColor(cv::Mat &srcImg){
    if(srcImg.empty()){
        throw std::invalid_argument("传递参数错误!");
    }

    cv::Mat grayImg;
    cv::cvtColor(srcImg, grayImg, cv::COLOR_RGB2GRAY);
    return grayImg;
}

cv::Mat CDistortionCorrection::imgFilte(cv::Mat &img){
    if(img.empty()){
        throw std::invalid_argument("传递参数错误!");
    }

    cv::Mat gaussian;
	GaussianBlur(img, gaussian, cv::Size(3, 3), 0);
    return gaussian;
}

int CDistortionCorrection::_threshold(cv::Mat &srcImg, cv::Mat &thresh, int minThreshold, int maxThreshold){
    if(srcImg.empty()){
        throw std::invalid_argument("传递参数错误!");
    }
    
	int value = threshold(srcImg, thresh, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
    return value;
}

cv::Mat CDistortionCorrection::morphologicalOperations(cv::Mat &grayImg){
    if(grayImg.empty()){
        throw std::invalid_argument("传递参数错误!");
    }
    
    /**创建一个一个3*3的卷积核 */
    cv::Mat conv2d = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));

    /**图像的开操作 */
    cv::Mat openImg;
    cv::morphologyEx(grayImg, openImg, cv::MORPH_OPEN, conv2d);

    /**创建一个7*7的卷积核 */
    cv::Mat conv2d_2 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 7));

    cv::Mat closeImg;
    cv::morphologyEx(openImg, closeImg, cv::MORPH_CLOSE, conv2d_2);

    return closeImg;
}

std::vector<std::vector<cv::Point>> CDistortionCorrection::_findContours(cv::Mat &closeImg){
    if(closeImg.empty()){
        throw std::invalid_argument("传递参数错误!");
    }

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(closeImg, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    return contours;
}

void CDistortionCorrection::plotCoutours(cv::Mat &srcImg, std::vector<std::vector<cv::Point>> &contours){
    if((srcImg.empty()) && (contours.size() < 0)){
        throw std::invalid_argument("传递参数错误!");
    }

    std::cout << "数组尺寸：" << contours[0].size() << std::endl;
    for(int i = 0; i < contours[0].size(); i++){
        std::cout << "(x, y)--> (" <<  contours[0][i].x << ", " << contours[0][i].y << ")" <<std::endl;
    }

    // 在 binaryImg 上绘制前 16 个点，用红色标记
    for (int i = 0; i < contours[0].size(); i++) {
        cv::Point pt = contours[0][i];
        cv::circle(srcImg, pt, 3, cv::Scalar(0, 0, 255), -1); // 绘制半径为 3 的红色圆点
    }

    // 显示结果图像
    cv::namedWindow("Binary Image with Points", cv::WINDOW_AUTOSIZE);
    cv::imshow("Binary Image with Points", srcImg);
    cv::waitKey(20);
}

void CDistortionCorrection::plotCoutours(cv::Mat &srcImg, std::map<std::string, cv::Point> points){
    if((srcImg.empty()) && (points.size() == 0)){
        throw std::invalid_argument("传递参数错误!");
    }

    /**绘制轮廓 */
    cv::circle(srcImg, points["left_top"], 10, cv::Scalar(0, 0, 255), -1);
    cv::circle(srcImg, points["left_bottom"], 10, cv::Scalar(0, 0, 255), -1);
    cv::circle(srcImg, points["right_top"], 10, cv::Scalar(0, 0, 255), -1);
    cv::circle(srcImg, points["right_bottom"], 10, cv::Scalar(0, 0, 255), -1);

    // 显示结果图像
    cv::namedWindow("circle", cv::WINDOW_AUTOSIZE);
    cv::imshow("circle", srcImg);
    cv::waitKey(20);
}

void CDistortionCorrection::plotCoutours(cv::Mat &srcImg){
    if(srcImg.empty()){
        throw std::invalid_argument("传递参数错误!");
    }
    cv::namedWindow("dstImg", cv::WINDOW_AUTOSIZE);
    cv::imshow("dstImg", srcImg);
    cv::waitKey(20);
}

std::vector<cv::Point> CDistortionCorrection::contoursApproach(std::vector<std::vector<cv::Point>> contours){
    if((contours.size() == 0) && (contours[0].size() == 0)){
        throw std::invalid_argument("传递参数错误!");
    }

    std::vector<std::vector<cv::Point>> conPoly(contours.size());
    std::vector<cv::Point> srcPts;
    /**遍历轮廓 */
    for(int i = 0; i < contours.size(); i++){
        /**计算每个轮廓的面积 */
        double area = cv::contourArea(contours[i]);

        /**如果轮廓的面积没有达标 */
        if(area > 10000){

            /**计算轮廓的周长 */
            double peri = cv::arcLength(contours[i], true);

            /**轮廓逼近 */
            cv::approxPolyDP(contours[i], conPoly[i], 0.02 * peri, true);
            
            /**保存轮廓的四个角点 */
            srcPts = {conPoly[i][0], conPoly[i][1], conPoly[i][2], conPoly[i][3]};
        }
    }
    return srcPts;
}

std::map<std::string, cv::Point> CDistortionCorrection::contoursPointsSort(cv::Mat &srcImg, std::vector<cv::Point> srcPts){
    if((srcPts.size() == 0) && (srcImg.empty())){
        throw std::invalid_argument("传递参数错误!");
    }

    /**获取图像的尺寸 */
    int weidth = srcImg.cols;
    int height = srcImg.rows;

    std::map<std::string, cv::Point> coordinatePoint;
    
    /**遍历角点坐标 */
    for(int i = 0; i < srcPts.size(); i++){
        if((srcPts.at(i).x < weidth / 2) && (srcPts.at(i).y < height / 2)){
            coordinatePoint.insert(std::make_pair("left_top", srcPts[i]));
        }
        if((srcPts.at(i).x < weidth / 2) && (srcPts.at(i).y > height / 2)){
            coordinatePoint.insert(std::make_pair("left_bottom", srcPts[i]));
        }
        if((srcPts.at(i).x > weidth / 2) && (srcPts.at(i).y < height / 2)){
            coordinatePoint.insert(std::make_pair("right_top", srcPts[i]));
        }
        if((srcPts.at(i).x > weidth / 2) && (srcPts.at(i).y > height / 2)){
            coordinatePoint.insert(std::make_pair("right_bottom", srcPts[i]));
        }
    }
    return coordinatePoint;
}

cv::Mat CDistortionCorrection::imgCorrection(cv::Mat &srcImg, std::map<std::string, cv::Point> &point){
    if((point.size() == 0) && (srcImg.empty())){
        throw std::invalid_argument("传递参数错误!");
    }

    /**获取最大宽度和最大高度 */
    double upWidth = cv::norm(point["left_top"] - point["right_top"]);
    double downWidth = cv::norm(point["left_bottom"] - point["right_bottom"]);
    double leftHeight = cv::norm(point["left_top"] - point["left_bottom"]);
    double rightHeight = cv::norm(point["right_top"] - point["right_bottom"]);

    double maxWidth = cv::max(upWidth, downWidth);
    double maxHeigth = cv::max(leftHeight, rightHeight);
    std::cout << "(" << maxWidth << ", " << maxHeigth << ")" << std::endl;

    /**原始矩阵 */
    cv::Point2f srcAffinePts[4] = {
        cv::Point2f(point["left_top"]),             // 左上
        cv::Point2f(point["right_top"]),             // 右上
        cv::Point2f(point["right_bottom"]),         // 右下
        cv::Point2f(point["left_bottom"])           // 左下
    };

    /**目标矩阵 */
    cv::Point2f dstAffinePts[4] = {
        cv::Point2f(0, 0),                          // 左上
        cv::Point2f(maxWidth, 0),                   // 右上
        cv::Point2f(maxWidth, maxHeigth),           // 右下
        cv::Point2f(0, maxHeigth)                   // 左下
    };

    /**计算透视变换矩阵 */
    cv::Mat perspectiveMatrix = cv::getPerspectiveTransform(srcAffinePts, dstAffinePts);

    /**应用透视变化矩阵 */
    cv::Mat dstImg;
    cv::warpPerspective(srcImg, dstImg, perspectiveMatrix, cv::Size(456, 261));
    return dstImg;
}

/*************************************************************************
* 改动历史纪录：
Revision 1.0, 2025-04-10, lium
describe: 初始创建.
*************************************************************************/