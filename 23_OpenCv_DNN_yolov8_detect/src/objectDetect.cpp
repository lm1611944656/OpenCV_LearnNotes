/*************************************************************************
 *
 *   Copyright (C), 2017-2027, BPG. Co., Ltd.
 *
 *   文件名称: objectdetect.cpp
 *   软件模块: 目标检测
 *   版 本 号: 1.0
 *   生成日期: 2025-07-17
 *   作    者: lium
 *   功    能: 图像检测源文件定义
 *
 ************************************************************************/

#include "objectDetect.h"

// 常量
#define INPUT_WIDTH 			640
#define INPUT_HEIGHT 			640
#define SCORE_THRESHOLD 		0.5
#define NMS_THRESHOLD 			0.45
#define CONFIDENCE_THRESHOLD 	0.45

CObjectDetect::CObjectDetect(const std::string &modelPath, const std::string &classFilePath)
	: modelPath_(modelPath), classFilePath_(classFilePath)
{
}

void CObjectDetect::letterBox(const cv::Mat &image, cv::Mat &outImage,
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

void CObjectDetect::scaleBoxes(cv::Rect &box, cv::Size size)
{
	float gain = std::min(INPUT_WIDTH * 1.0 / size.width, INPUT_HEIGHT * 1.0 / size.height);
	int pad_w = (INPUT_WIDTH - size.width * gain) / 2;
	int pad_h = (INPUT_HEIGHT - size.height * gain) / 2;
	box.x -= pad_w;
	box.y -= pad_h;
	box.x /= gain;
	box.y /= gain;
	box.width /= gain;
	box.height /= gain;
}

void CObjectDetect::nms(std::vector<cv::Rect> & boxes, std::vector<float> & scores, float score_threshold, float nms_threshold, std::vector<int> & indices)
{
	assert(boxes.size() == scores.size());

	struct BoxScore
	{
		cv::Rect box;
		float score;
		int id;
	};
	std::vector<BoxScore> boxes_scores;
	for (size_t i = 0; i < boxes.size(); i++)
	{
		BoxScore box_conf;
		box_conf.box = boxes[i];
		box_conf.score = scores[i];
		box_conf.id = i;
		if (scores[i] > SCORE_THRESHOLD)	boxes_scores.push_back(box_conf);
	}

	std::sort(boxes_scores.begin(), boxes_scores.end(), [](BoxScore a, BoxScore b) { return a.score > b.score; });

	std::vector<float> area(boxes_scores.size());
	for (int i = 0; i < (int)boxes_scores.size(); ++i)
	{
		area[i] = boxes_scores[i].box.width * boxes_scores[i].box.height;
	}

	std::vector<bool> isSuppressed(boxes_scores.size(), false);
	for (size_t i = 0; i < boxes_scores.size(); ++i)
	{
		if (isSuppressed[i])  continue;
		for (size_t j = i + 1; j < boxes_scores.size(); ++j)
		{
			if (isSuppressed[j])  continue;

			float x1 = (std::max)(boxes_scores[i].box.x, boxes_scores[j].box.x);
			float y1 = (std::max)(boxes_scores[i].box.y, boxes_scores[j].box.y);
			float x2 = (std::min)(boxes_scores[i].box.x + boxes_scores[i].box.width, boxes_scores[j].box.x + boxes_scores[j].box.width);
			float y2 = (std::min)(boxes_scores[i].box.y + boxes_scores[i].box.height, boxes_scores[j].box.y + boxes_scores[j].box.height);
			float w = (std::max)(0.0f, x2 - x1);
			float h = (std::max)(0.0f, y2 - y1);
			float inter = w * h;
			float ovr = inter / (area[i] + area[j] - inter);

			if (ovr >= NMS_THRESHOLD)  isSuppressed[j] = true;
		}
	}

	for (int i = 0; i < (int)boxes_scores.size(); ++i)
	{
		if (!isSuppressed[i])	indices.push_back(boxes_scores[i].id);
	}
}

void CObjectDetect::readClassLabel(void)
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

void CObjectDetect::readImg(const std::string &srcImgPath)
{
	if (srcImgPath.empty())
	{
		throw std::invalid_argument("传递参数错误!");
	}
	srcImg_ = cv::imread(srcImgPath, cv::WINDOW_AUTOSIZE);
}

cv::Mat CObjectDetect::preprocess()
{
	if (this->srcImg_.empty())
	{
		throw std::invalid_argument("传递参数错误!");
	}
	cv::Vec4d params;
	cv::Mat letterbox;
	cv::Mat blob;

	letterBox(this->srcImg_, letterbox, params, cv::Size(INPUT_WIDTH, INPUT_HEIGHT));
	cv::dnn::blobFromImage(letterbox, blob, 1. / 255., cv::Size(INPUT_WIDTH, INPUT_HEIGHT), cv::Scalar(), true, false);
	return blob;
}

std::vector<cv::Mat> CObjectDetect::modelInference(cv::Mat &blob, TDeviceType_t deviceType)
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

std::vector<TDetectResult_t> CObjectDetect::postprocess(std::vector<cv::Mat> &netOut)
{
	std::vector<TDetectResult_t> detectResult;
	std::vector<int> class_ids;		// res-class_id
	std::vector<float> confidences; // res-conf
	std::vector<cv::Rect> boxes;	// res-box

	/**获取网络输出*/
	cv::Mat output0 = cv::Mat(cv::Size(netOut[0].size[2], netOut[0].size[1]), CV_32F, (float *)netOut[0].data).t(); //[bs,84,8400]=>[bs,8400,84]

	int net_width = output0.cols;
	int rows = output0.rows;
	// float *pData = (float *)output0.data;

	for (int item = 0; item < rows; ++item)
	{
		/**获取行指针 */
		float *pData = output0.ptr<float>(item);

		/**获取类别得分*/
		cv::Mat scores(1, className_.size(), CV_32FC1, pData + 4);

		cv::Point class_id_point; // 最大得分的类别索引
		double max_class_score;	  // 最大得分值

		/**寻找最大得分及其对应类别 */
		cv::minMaxLoc(scores, 0, &max_class_score, 0, &class_id_point);

		/**如果最大得分大于设定的阈值，则保留该检测框 */
		if (max_class_score >= SCORE_THRESHOLD)
		{
			float x = *(pData + 0);
			float y = *(pData + 1);
			float w = *(pData + 2);
			float h = *(pData + 3);
			int left = int(x - 0.5 * w);
			int top = int(y - 0.5 * h);
			int width = int(w);
			int height = int(h);

			cv::Rect box = cv::Rect(left, top, width, height);
			scaleBoxes(box, this->srcImg_.size());

			class_ids.push_back(class_id_point.x);
			confidences.push_back(max_class_score);
			boxes.push_back(box);
		}

		/**指针偏移 */
		pData += net_width;
	}

	/**NMS */
	std::vector<int> nms_result;
	nms(boxes, confidences, SCORE_THRESHOLD, NMS_THRESHOLD, nms_result);

	for (unsigned char i = 0; i < (unsigned char)nms_result.size(); ++i)
	{
		int idx = nms_result[i];

		TDetectResult_t result;
		result.id = class_ids[idx];					   // 第几个类别(类别ID)
		result.confidence = confidences[idx];		   // 类别置信度
		result.box = boxes[idx];					   // 类别检测框
		result.className = className_[class_ids[idx]]; // 类别名称
		detectResult.push_back(result);

		/**绘制结果 */
		std::string label = className_[class_ids[idx]] + ":" + cv::format("%.2f", confidences[idx]);
		drawResult(this->srcImg_, label, result.box);
	}
	return detectResult;
}

void CObjectDetect::drawResult(cv::Mat &srcImg, std::string label, cv::Rect box)
{
	cv::rectangle(srcImg, box, cv::Scalar(255, 0, 0), 2);
	cv::putText(srcImg, label, cv::Point(box.x, box.y), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 255), 1);
	cv::imshow("srcImg", srcImg);
}

void CObjectDetect::modelDetect(const std::string &imgPath)
{
	readClassLabel();
	readImg(imgPath);
	cv::Mat blob = preprocess();
	std::vector<cv::Mat> netOut = modelInference(blob, CPU);
	std::vector<TDetectResult_t> detectResult = postprocess(netOut);
	if(detectResult.size() == 0){
		std::cout << "detect result null" << std::endl;
	}else{
		std::cout << "detect result size:" << detectResult.size() << std::endl;
		for(int i = 0; i < detectResult.size(); i++){
			std::cout << "obj name: " << detectResult[i].className << std::endl;
		}
	}
}

/*************************************************************************
 * 改动历史纪录：
 * Revision 1.0, 2025-07-17, lium
 * describe: 初始创建.
 *************************************************************************/