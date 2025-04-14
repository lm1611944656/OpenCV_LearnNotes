/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: distortioncorrection.h
*   软件模块: 应用模块
*   版 本 号: 1.0
*   生成日期: 2025-04-10
*   作    者: lium
*   功    能: 图像畸变校正
* 
*************************************************************************/

#ifndef __DISTORTIONCORRECTION_H__
#define __DISTORTIONCORRECTION_H__

#include <iostream>
#include <opencv2/opencv.hpp>

class CDistortionCorrection{
public:
    explicit CDistortionCorrection(const std::string &imgPath);
    ~CDistortionCorrection();

public:
    cv::Mat getBinaryImg(void);

private:
    void deInit(void);
    void formInit(void);

    /**读取图像*/
    cv::Mat readImg(void);

    /**通道压缩*/
    cv::Mat _cvtColor(cv::Mat &srcImg);

    /**滤波*/
    cv::Mat imgFilte(cv::Mat &img);

    /**二值化*/
    int _threshold(cv::Mat &srcImg, cv::Mat &thresh, int minThreshold, int maxThreshold);

    /**形态学操作*/
    cv::Mat morphologicalOperations(cv::Mat &grayImg);

    /**查找轮廓*/
    std::vector<std::vector<cv::Point>> _findContours(cv::Mat &closeImg);

    /**轮廓逼近 */
    std::vector<cv::Point> contoursApproach(std::vector<std::vector<cv::Point>>);

    /**轮廓点排序 */
    std::map<std::string, cv::Point> contoursPointsSort(cv::Mat &srcImg, std::vector<cv::Point>);

    /**透视矫正*/
    cv::Mat imgCorrection(cv::Mat &srcImg, std::map<std::string, cv::Point> &point);

    /**绘制轮廓 */
    void plotCoutours(cv::Mat &srcImg, std::vector<std::vector<cv::Point>> &contors);
    void plotCoutours(cv::Mat &srcImg, std::map<std::string, cv::Point> points);
    void plotCoutours(cv::Mat &srcImg);
private:
    std::string _imgPath;
    cv::Mat binaryImg;
};


#endif /**__DISTORTIONCORRECTION_H__ */

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-04-10, lium
* describe: 初始创建.
*************************************************************************/