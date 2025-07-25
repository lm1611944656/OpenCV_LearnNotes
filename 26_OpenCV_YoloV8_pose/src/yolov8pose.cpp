/*************************************************************************
 *
 *   Copyright (C), 2017-2027, BPG. Co., Ltd.
 *
 *   文件名称: yolov8pose.cpp
 *   软件模块: 目标检测
 *   版 本 号: 1.0
 *   生成日期: 2025-07-23
 *   作    者: lium
 *   功    能: 关键点检测源文件定义
 *
 ************************************************************************/

#include "yolov8pose.h"

// 常量
#define INPUT_WIDTH 640
#define INPUT_HEIGHT 640
#define SCORE_THRESHOLD 0.5
#define NMS_THRESHOLD 0.45
#define CONFIDENCE_THRESHOLD 0.45

CYoloV8Pose::CYoloV8Pose(const std::string &modelPath, const std::string &classNamePath)
	: modelPath_(modelPath), classFilePath_(classNamePath)
{
}

CYoloV8Pose::~CYoloV8Pose()
{
}

void CYoloV8Pose::letterBox(const cv::Mat &image, cv::Mat &outImage,
							cv::Vec4d &params, //[ratio_x,ratio_y,dw,dh]
							const cv::Size &newShape,
							bool autoShape,
							bool scaleFill,
							bool scaleUp,
							int stride,
							const cv::Scalar &color)
{
	cv::Size shape = image.size();
	float r = std::min((float)newShape.height / (float)shape.height, (float)newShape.width / (float)shape.width);
	if (!scaleUp)
	{
		r = std::min(r, 1.0f);
	}

	float ratio[2]{r, r};
	int new_un_pad[2] = {(int)std::round((float)shape.width * r), (int)std::round((float)shape.height * r)};

	auto dw = (float)(newShape.width - new_un_pad[0]);
	auto dh = (float)(newShape.height - new_un_pad[1]);

	if (autoShape)
	{
		dw = (float)((int)dw % stride);
		dh = (float)((int)dh % stride);
	}
	else if (scaleFill)
	{
		dw = 0.0f;
		dh = 0.0f;
		new_un_pad[0] = newShape.width;
		new_un_pad[1] = newShape.height;
		ratio[0] = (float)newShape.width / (float)shape.width;
		ratio[1] = (float)newShape.height / (float)shape.height;
	}

	dw /= 2.0f;
	dh /= 2.0f;

	if (shape.width != new_un_pad[0] && shape.height != new_un_pad[1])
		cv::resize(image, outImage, cv::Size(new_un_pad[0], new_un_pad[1]));
	else
		outImage = image.clone();

	int top = int(std::round(dh - 0.1f));
	int bottom = int(std::round(dh + 0.1f));
	int left = int(std::round(dw - 0.1f));
	int right = int(std::round(dw + 0.1f));
	params[0] = ratio[0];
	params[1] = ratio[1];
	params[2] = left;
	params[3] = top;
	cv::copyMakeBorder(outImage, outImage, top, bottom, left, right, cv::BORDER_CONSTANT, color);
}

void CYoloV8Pose::scaleBoxes(cv::Rect &box)
{
	// 去掉填充偏移
	box.x -= static_cast<int>(params_[2]); // 左填充
	box.y -= static_cast<int>(params_[3]); // 上填充

	// 恢复到原始图像尺寸
	box.x = static_cast<int>(box.x / params_[0]);
	box.y = static_cast<int>(box.y / params_[1]);
	box.width = static_cast<int>(box.width / params_[0]);
	box.height = static_cast<int>(box.height / params_[1]);
}

void CYoloV8Pose::readClassLabel(void)
{
	std::ifstream fp(classFilePath_);
	if (!fp.is_open())
	{
		std::cout << "打开类别文件名失败！" << std::endl;
		printf("could not open file...\n");
		exit(-1);
	}
	std::string name;
	while (!fp.eof())
	{
		std::getline(fp, name);
		if (name.length())
			this->className_.push_back(name);
	}
	fp.close();
}

void CYoloV8Pose::readImg(const std::string &srcImgPath)
{
	if (srcImgPath.empty())
	{
		throw std::invalid_argument("传递参数错误!");
	}
	srcImg_ = cv::imread(srcImgPath, cv::WINDOW_AUTOSIZE);
}

cv::Mat CYoloV8Pose::preprocess()
{
	if (this->srcImg_.empty())
	{
		throw std::invalid_argument("传递参数错误!");
	}
	// cv::Vec4d params;
	cv::Mat letterbox;
	cv::Mat blob;

	letterBox(this->srcImg_, letterbox, params_, cv::Size(INPUT_WIDTH, INPUT_HEIGHT));
	cv::dnn::blobFromImage(letterbox, blob, 1. / 255., cv::Size(INPUT_WIDTH, INPUT_HEIGHT), cv::Scalar(), true, false);

	// cv::imshow("blob", letterbox);
	return blob;
}

std::vector<cv::Mat> CYoloV8Pose::modelInference(cv::Mat &blob, TDeviceType_t deviceType)
{
	if (blob.empty())
	{
		throw std::invalid_argument("传递参数错误!");
	}

	/**读取网络模型 */
	cv::dnn::Net model = cv::dnn::readNetFromONNX(this->modelPath_);
	if (model.empty())
	{
		throw std::runtime_error("无法加载模型文件: " + modelPath_);
	}

	/** 判断是是GPU和CPU */
	if (deviceType == GPU)
	{
		model.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
		model.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
	}
	else if (deviceType == CPU)
	{
		model.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
		model.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
	}

	/**模型设置输入 */
	model.setInput(blob);

	/**需要多个输出层 */
	std::vector<cv::Mat> netOutResult;
	model.forward(netOutResult, model.getUnconnectedOutLayersNames());
	return netOutResult;
}

std::vector<TOutputPose_t> CYoloV8Pose::postprocess(std::vector<cv::Mat> &netOut)
{
	std::vector<TOutputPose_t> keyPointsDetectResult;
	std::vector<std::vector<float>> kpss; // 关键点
	std::vector<float> confidences;		  // 置信度
	std::vector<cv::Rect> boxes;		  // res-box

	cv::Mat output0 = cv::Mat(cv::Size(netOut[0].size[2], netOut[0].size[1]), CV_32FC1, (float *)netOut[0].data).t();

	/**获取矩阵的信息 */
	int new_width = output0.cols;
	int new_high = output0.rows;
	// float *pMatrixData = (float *)output0.data;

	/**
	 * 获取关键点的数量
	 * 每行输出格式为 [x, y, w, h, score, ...keypoints]
	 * 例如人体姿势检测就是17个关键点，每个关键点为(x, y, v), 也就是一个关键点有三个信息
	 * 17 * 3 = 51
	 * 51 + 5
	 * output0的矩阵信息为：8400 * 56
	 */
	int num_keypoints = (new_width - 5) / 3;

	for (int row = 0; row < new_high; row++)
	{
		/**获取行指针 */
		float *pRow = output0.ptr<float>(row);

		/**判断类别得分 */
		float score = *(pRow + 4);
		if (score > CONFIDENCE_THRESHOLD)
		{
			/**获取anchor box */
			float x = *(pRow + 0);
			float y = *(pRow + 1);
			float w = *(pRow + 2);
			float h = *(pRow + 3);
			int left = static_cast<int>(x - 0.5 * w);
			int top = static_cast<int>(y - 0.5 * h);
			int width = static_cast<int>(w);
			int high = static_cast<int>(h);

			/**获取关键点信息 */
			std::vector<float> keyPoints;
			for (int k = 0; k < num_keypoints; ++k)
			{
				float x = *(pRow + 5 + k * 3 + 0); // pRow + 5 + 1*3 + 0 对应于第1个关键点的x坐标
				float y = *(pRow + 5 + k * 3 + 1);
				float v = *(pRow + 5 + k * 3 + 2);

				/**去除 letterbox padding 并还原到原始图像尺寸 */
				x = (x - params_[2]) / params_[0]; // params_[0] = ratio_x, params_[2] = pad_w
				y = (y - params_[3]) / params_[1]; // params_[1] = ratio_y, params_[3] = pad_h

				keyPoints.push_back(x);
				keyPoints.push_back(y);
				keyPoints.push_back(v);
			}
			// std::cout << "keyPoints size(): " << keyPoints.size() << std::endl;  // 应为 51

			/**创建一个box */
			cv::Rect box = cv::Rect(left, top, width, high);
			scaleBoxes(box);

			/**保存 */
			boxes.push_back(box);
			confidences.push_back(score);
			kpss.push_back(keyPoints);
		}
	}

	std::vector<int> nms_result;
	cv::dnn::NMSBoxes(boxes, confidences, SCORE_THRESHOLD, NMS_THRESHOLD, nms_result);
	for (unsigned char i = 0; i < (unsigned char)nms_result.size(); ++i)
	{
		int idx = nms_result[i];

		TOutputPose_t outPoseResult;

		/**矩形框 */
		outPoseResult.box = boxes[idx];

		/**置信度 */
		outPoseResult.confidence = confidences[idx];

		/**关键点 */
		outPoseResult.kps = kpss[idx];

		/**保存 */
		keyPointsDetectResult.push_back(outPoseResult);

		/**绘制结果 */
		/**类别标签 "person"*/
		std::string label = "person:" + cv::format("%.2f", confidences[idx]);
		// drawResult(this->srcImg_, label, outPoseResult.box);
	}

	return keyPointsDetectResult;
}

void CYoloV8Pose::drawResult(cv::Mat &srcImg, std::string label, cv::Rect box)
{
	cv::rectangle(srcImg, box, cv::Scalar(255, 0, 0), 1);
	cv::putText(srcImg, label, cv::Point(box.x, box.y), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 255), 1);
	cv::imshow("srcImg", srcImg);
}

void CYoloV8Pose::drawPose(cv::Mat &img, const std::vector<TOutputPose_t> &keyPointsDetectResult,
						   const std::vector<std::vector<unsigned int>> &skeleton,
						   const std::vector<std::vector<unsigned int>> &kps_colors,
						   const std::vector<std::vector<unsigned int>> &limb_colors)
{
	const int num_point = 17;
	for (auto &result : keyPointsDetectResult)
	{
		int left, top, width, height;
		left = result.box.x;
		top = result.box.y;
		width = result.box.width;
		height = result.box.height;

		//        printf("x: %d  y:%d  w:%d  h%d\n",(int)left, (int)top, (int)result.box.width, (int)result.box.height);

		// 框出目标
		cv::rectangle(img, result.box, cv::Scalar(255, 0, 0), 1, 8);

		// 在目标框左上角标识目标类别以及概率
		std::string label = "person:" + std::to_string(result.confidence);
		int baseLine;
		cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
		top = cv::max(top, labelSize.height);
		cv::putText(img, label, cv::Point(left, top), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 2);

		// 连线
		auto &kps = result.kps;
		//        cout << "该目标关键点：" << kps.size() << endl;
		for (int k = 0; k < num_point + 2; k++)
		{ // 不要设置为>0.5f ,>0.0f显示效果比较好
			// 关键点绘制
			if (k < num_point)
			{
				int kps_x = std::round(kps[k * 3]);
				int kps_y = std::round(kps[k * 3 + 1]);
				float kps_s = kps[k * 3 + 2];

				//                printf("x:%d y:%d s:%f\n", kps_x, kps_y, kps_s);

				if (kps_s > 0.0f)
				{
					cv::Scalar kps_color = cv::Scalar(kps_colors[k][0], kps_colors[k][1], kps_colors[k][2]);
					cv::circle(img, {kps_x, kps_y}, 5, kps_color, -1);
				}
			}

			auto &ske = skeleton[k];
			int pos1_x = std::round(kps[(ske[0] - 1) * 3]);
			int pos1_y = std::round(kps[(ske[0] - 1) * 3 + 1]);

			int pos2_x = std::round(kps[(ske[1] - 1) * 3]);
			int pos2_y = std::round(kps[(ske[1] - 1) * 3 + 1]);

			float pos1_s = kps[(ske[0] - 1) * 3 + 2];
			float pos2_s = kps[(ske[1] - 1) * 3 + 2];

			if (pos1_s > 0.0f && pos2_s > 0.0f)
			{ // 不要设置为>0.5f ,>0.0f显示效果比较好
				cv::Scalar limb_color = cv::Scalar(limb_colors[k][0], limb_colors[k][1], limb_colors[k][3]);
				cv::line(img, {pos1_x, pos1_y}, {pos2_x, pos2_y}, limb_color);
			}

			/***************************************跌倒检测*******************************/
			float pt5_x = kps[5 * 3];
			float pt5_y = kps[5 * 3 + 1];
			float pt6_x = kps[6 * 3];
			float pt6_y = kps[6 * 3 + 1];
			float center_up_x = (pt5_x + pt6_x) / 2.0f;
			float center_up_y = (pt5_y + pt6_y) / 2.0f;
			cv::Point center_up = cv::Point((int)center_up_x, (int)center_up_y);

			float pt11_x = kps[11 * 3];
			float pt11_y = kps[11 * 3 + 1];
			float pt12_x = kps[12 * 3];
			float pt12_y = kps[12 * 3 + 1];
			float center_down_x = (pt11_x + pt12_x) / 2.0f;
			float center_down_y = (pt11_y + pt12_y) / 2.0f;
			cv::Point center_down = cv::Point((int)center_down_x, (int)center_down_y);

			float right_angle_point_x = center_down_x;
			float righ_angle_point_y = center_up_y;
			cv::Point right_angl_point = cv::Point((int)right_angle_point_x, (int)righ_angle_point_y);

			float a = abs(right_angle_point_x - center_up_x);
			float b = abs(center_down_y - righ_angle_point_y);

			float tan_value = a / b;
			float Pi = acos(-1);
			float angle = atan(tan_value) * 180.0f / Pi;
			std::string angel_label = "angle: " + std::to_string(angle);
			putText(img, angel_label, cv::Point(left, top - 40), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 2);

			if (angle > 60.0f || center_down_y <= center_up_y || (double)width / height > 5.0f / 3.0f) // 宽高比小于0.6为站立，大于5/3为跌倒
			{
				std::string fall_down_label = "person fall down!!!!";
				putText(img, fall_down_label, cv::Point(left, top - 20), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 2);

				printf("angel:%f width/height:%f\n", angle, (double)width / height);
			}

			cv::line(img, center_up, center_down,
					 cv::Scalar(0, 0, 255), 2, 8);
			cv::line(img, center_up, right_angl_point,
					 cv::Scalar(0, 0, 255), 2, 8);
			cv::line(img, right_angl_point, center_down,
					 cv::Scalar(0, 0, 255), 2, 8);

			/***************************************跌倒检测*******************************/
		}
	}
}

const std::vector<std::vector<unsigned int>> KPS_COLORS =
	{{0, 255, 0},
	 {0, 255, 0},
	 {0, 255, 0},
	 {0, 255, 0},
	 {0, 255, 0},
	 {255, 128, 0},
	 {255, 128, 0},
	 {255, 128, 0},
	 {255, 128, 0},
	 {255, 128, 0},
	 {255, 128, 0},
	 {51, 153, 255},
	 {51, 153, 255},
	 {51, 153, 255},
	 {51, 153, 255},
	 {51, 153, 255},
	 {51, 153, 255}};

const std::vector<std::vector<unsigned int>> SKELETON = {{16, 14},
														 {14, 12},
														 {17, 15},
														 {15, 13},
														 {12, 13},
														 {6, 12},
														 {7, 13},
														 {6, 7},
														 {6, 8},
														 {7, 9},
														 {8, 10},
														 {9, 11},
														 {2, 3},
														 {1, 2},
														 {1, 3},
														 {2, 4},
														 {3, 5},
														 {4, 6},
														 {5, 7}};

const std::vector<std::vector<unsigned int>> LIMB_COLORS = {{51, 153, 255},
															{51, 153, 255},
															{51, 153, 255},
															{51, 153, 255},
															{255, 51, 255},
															{255, 51, 255},
															{255, 51, 255},
															{255, 128, 0},
															{255, 128, 0},
															{255, 128, 0},
															{255, 128, 0},
															{255, 128, 0},
															{0, 255, 0},
															{0, 255, 0},
															{0, 255, 0},
															{0, 255, 0},
															{0, 255, 0},
															{0, 255, 0},
															{0, 255, 0}};

void CYoloV8Pose::modelDetect(const std::string &imgPath)
{
	readClassLabel();
	readImg(imgPath);
	cv::Mat blob = preprocess();
	std::vector<cv::Mat> netOut = modelInference(blob, CPU);
	std::vector<TOutputPose_t> keyPointsDetectResult = postprocess(netOut);
	std::cout << "keyPointsDetectResult 的数量：" << keyPointsDetectResult.size() << std::endl;

	std::cout << keyPointsDetectResult[0].kps.size() << std::endl; // 51

	drawPose(srcImg_, keyPointsDetectResult, SKELETON, KPS_COLORS, LIMB_COLORS);
	cv::imshow("Detected Keypoints", srcImg_);
}

/*************************************************************************
 * 改动历史纪录：
 * Revision 1.0, 2025-07-23, lium
 * describe: 初始创建.
 *************************************************************************/