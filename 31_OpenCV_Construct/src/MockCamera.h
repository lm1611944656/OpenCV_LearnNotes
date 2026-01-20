/************************************************************************
*
*   Copyright (C), 2012-2020, GTBS. Co., Ltd.
*
*   软件模块 ：基础框架
*   功   能	：模拟相机实现类（生成随机图像，用于测试）
*
*************************************************************************/
#ifndef __MOCKCAMERA_H__
#define __MOCKCAMERA_H__

#include <opencv2/opencv.hpp>
#include "base_camera.h"

// 模拟相机实现类（生成随机图像，用于测试）
class MockCamera : public BaseCamera {
public:
	MockCamera(const std::string& camera_id, const cv::Size& resolution = cv::Size(640, 480))
		: BaseCamera(camera_id),
		resolution_(resolution) {}

	~MockCamera() override {
		// 析构函数无需重复调用closeCamera，基类析构会自动调用
	}

	// 模拟初始化
	bool initCamera() override {
		if (initialized_) {
			return true;
		}

		setInitialized(true);
		std::cout << "[INFO] Mock camera " << camera_id_ << " initialized!" << std::endl;
		return true;
	}

	// 生成随机图像
	cv::Mat captureSingleFrame() override {
		if (!initialized_) {
			return cv::Mat();
		}

		// 生成随机图像
		cv::Mat frame(resolution_, CV_8UC3);
		cv::randu(frame, cv::Scalar(0, 0, 0), cv::Scalar(255, 255, 255));

		// 绘制相机ID标识
		cv::putText(frame, "Camera: " + camera_id_,
			cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX,
			1.0, cv::Scalar(0, 255, 0), 2);

		return frame;
	}

	// 模拟关闭（重写基类虚函数）
	void closeCamera() override {
		setInitialized(false);
		std::cout << "[INFO] Mock camera " << camera_id_ << " closed!" << std::endl;
	}

private:
	cv::Size resolution_; // 分辨率
};


#endif /**__MOCKCAMERA_H__ */
/*************************************************************************
* 改动历史纪录：
Revision 1.0, 2026-01-20, lium
describe: 初始创建.
*************************************************************************/