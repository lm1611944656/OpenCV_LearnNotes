#ifndef __NETWORKINTERFACEDETECT_H__
#define __NETWORKINTERFACEDETECT_H__

#include <opencv2/opencv.hpp>
#include <iostream>


class CNetworkInterfaceDetect {
public:
    explicit CNetworkInterfaceDetect(const std::string &imgPath);
    ~CNetworkInterfaceDetect(void);

private:
    void deInit(void);

    void initForm(void);

    /**读取图像 */
    cv::Mat readImg(void);

    /**图像空间变化 */
    cv::Mat imgBGR2HSV(cv::Mat &srcImg);

    /**图像形态学操作 */
    cv::Mat _dilate(cv::Mat &binaryImg);

    /**图像修复(去除干扰颜色) */
    cv::Mat imgInpainting(cv::Mat &srcImg, cv::Mat &binaryImg);

    /**通道压缩 */
    cv::Mat imgBGR2Gray(cv::Mat &arcImg);

    /**图像滤波 */

    /**图像二值化 */
    cv::Mat binaryImg(cv::Mat &srcImg);

    /**图像形态学操作 */

    /**轮廓发现 */

    /**轮廓提取 */

    /**绘制图像 */
    void plotImg(cv::Mat &srcImg, const std::string &winName);

private:
    const std::string _imgPath;
};

#endif

