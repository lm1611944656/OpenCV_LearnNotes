/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: objectdetection.cpp
*   软件模块: 目标检测
*   版 本 号: 1.0
*   生成日期: 2025-05-29
*   作    者: lium
*   功    能: 图像检测源文件定义
* 
************************************************************************/

#include "objectdetection.h"
#include <fstream>

#define LINEWIDTH    1          // 绘制矩形框的线宽

const float CObjDetection::INPUT_WIDTH = 640.0F;
const float CObjDetection::INPUT_HEIGHT = 640.0F;
const float CObjDetection::SCORE_THRESHOLD = 0.5;
const float CObjDetection::NMS_THRESHOLD = 0.5;
const float CObjDetection::CONFIDENCE_THRESHOLD = 0.5;

CObjDetection::CObjDetection(const std::string &modelFile, std::string &classFile)
:modelFile_(modelFile), classFile_(classFile), x_factor(0.0), y_factor(0.0), startTick_(0)
{
    readClassLabel();
}

CObjDetection::~CObjDetection()
{

}

std::vector<TDetectResult> CObjDetection::imgDetectTask(std::string &imgPath)
{
    /**读取图像 */
    this->readImg(imgPath);

    /**开始计数(统计检测一张图像的耗时) */
    startTick_ = cv::getTickCount();

    /**预处理 */
    cv::Mat blob = this->preprocess();

    /**模型推理 */
    cv::Mat preds =  this->modelInference(blob, CPU);

    /**后处理 */
	std::vector<TDetectResult> result;
	this->postprocess(preds, this->srcImg_, &result);

	// for (const auto& detection : result) {
	// 	// std::cout << "Probability outside111111111:" << detection.probability << std::endl;
	// 	// std::cout << "检测结果为：" << std::endl;
	// 	std::cout << "Detected outside: " << detection.className << std::endl;
	// 	std::cout << "Probability outside: " << detection.probability;
	// 	std::cout << ", Box outside: (" << detection.box.x << ", " << detection.box.y << ", " 
	// 			<< detection.box.width << ", " << detection.box.height << ")" << std::endl;
	// }
    return result;
}

void CObjDetection::readClassLabel(void)
{
	std::ifstream fp(this->classFile_);
	if (!fp.is_open())
	{
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

void CObjDetection::readImg(const std::string &imgPath)
{
    if(imgPath.empty()){
        throw std::invalid_argument("传递参数错误!");
    }
    this->srcImg_ = cv::imread(imgPath);
}

cv::Mat CObjDetection::preprocess(void)
{
    /**数据预处理 */
    int w = this->srcImg_.cols;
	int h = this->srcImg_.rows;
	int _max = std::max(h, w);
	cv::Mat image = cv::Mat::zeros(cv::Size(_max, _max), CV_8UC3);
	cv::Rect roi(0, 0, w, h);
	this->srcImg_.copyTo(image(roi));

    /**保存缩放因子 */
    this->x_factor = image.cols / CObjDetection::INPUT_WIDTH;
	this->y_factor = image.rows / CObjDetection::INPUT_HEIGHT;

    /**tensor变化 */
    cv::Mat blob = cv::dnn::blobFromImage(image, 1 / 255.0, cv::Size(CObjDetection::INPUT_WIDTH, CObjDetection::INPUT_HEIGHT), cv::Scalar(0, 0, 0), true, false);
    return blob;
}

cv::Mat CObjDetection::modelInference(cv::Mat &tensor4D, TDeviceType deviceType)
{
    if(tensor4D.empty()){
        throw std::invalid_argument("传递参数错误!");
    }

    /**读取网络模型 */
    cv::dnn::Net model = cv::dnn::readNetFromONNX(this->modelFile_);
    if (model.empty()) {
        throw std::runtime_error("无法加载模型文件: " + modelFile_);
    }

    /** 判断是是GPU和CPU */
    if (deviceType == GPU) {
        model.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
        model.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
    } else if(deviceType == CPU){
        model.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        model.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    }

    /**模型设置输入 */
    model.setInput(tensor4D);

    /**需要多个输出层 */
    //std::vector<std::string> output_layer_names{ "output0", "output1" }; 

    /**只有一个输出层 */
    cv::Mat preds = model.forward();
    return preds;
}

void CObjDetection::postprocess(cv::Mat &preds, \
	cv::Mat &srcImg, std::vector<TDetectResult> *result)
{
    std::vector<cv::Scalar> colors;
	colors.push_back(cv::Scalar(0, 255, 0));
	colors.push_back(cv::Scalar(0, 255, 255));
	colors.push_back(cv::Scalar(255, 255, 0));
	colors.push_back(cv::Scalar(255, 0, 0));
	colors.push_back(cv::Scalar(0, 0, 255));

    cv::Mat det_output(preds.size[1], preds.size[2], CV_32F, preds.ptr<float>());

	std::vector<cv::Rect> boxes;
	std::vector<int> classIds;
	std::vector<float> confidences;
	for (int i = 0; i < det_output.rows; i++) {
		float confidence = det_output.at<float>(i, 4);
		if (confidence < 0.25) {
			continue;
		}
		cv::Mat classes_scores = det_output.row(i).colRange(5, preds.size[2]);
		cv::Point classIdPoint;
		double score;
		minMaxLoc(classes_scores, 0, &score, 0, &classIdPoint);

		// 置信度 0～1之间
		if (score > CObjDetection::CONFIDENCE_THRESHOLD)
		{
			float cx = det_output.at<float>(i, 0);
			float cy = det_output.at<float>(i, 1);
			float ow = det_output.at<float>(i, 2);
			float oh = det_output.at<float>(i, 3);
			int x = static_cast<int>((cx - 0.5 * ow) * this->x_factor);
			int y = static_cast<int>((cy - 0.5 * oh) * this->y_factor);
			int width = static_cast<int>(ow * this->x_factor);
			int height = static_cast<int>(oh * this->y_factor);
			cv::Rect box;
			box.x = x;
			box.y = y;
			box.width = width;
			box.height = height;

			boxes.push_back(box);
			classIds.push_back(classIdPoint.x);
			confidences.push_back(score);
		}
	}

	/**保存检查结果 */
    // std::vector<TDetectResult> detectResultVector;
    // detectResultVector.clear();

	// NMS
	std::vector<int> indexes;
	cv::dnn::NMSBoxes(boxes, confidences, CObjDetection::SCORE_THRESHOLD, CObjDetection::NMS_THRESHOLD, indexes);
	for (size_t i = 0; i < indexes.size(); i++) {
		int index = indexes[i];
		int idx = classIds[index];

		// 获取当前边界框的坐标
		cv::Rect box = boxes[index];
		int x = box.x;
		int y = box.y;
		int w = box.width;
		int h = box.height;

		// 绘制矩形框
		cv::rectangle(this->srcImg_, box, colors[idx % 5], 2, 8);

		// 绘制标签背景
		cv::rectangle(this->srcImg_,
					cv::Point(boxes[index].tl().x, boxes[index].tl().y - 40), // 调整高度以容纳更多文本
					cv::Point(boxes[index].br().x, boxes[index].tl().y),
					cv::Scalar(255, 255, 255), -1);

		// 构造标签文本，包括类别名称和置信度
		std::string label = className_[idx] + ":" + cv::format("%.2f", confidences[index]);

		// 绘制标签文本
		cv::putText(this->srcImg_, label,
					cv::Point(boxes[index].tl().x, boxes[index].tl().y - 10),
					cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));

		TDetectResult detectResult;
        detectResult.box = box;
		detectResult.probability = std::string(cv::format("%.2f", confidences[index]));
		detectResult.className =  className_[idx];
		
		// char buf[16];	// 类别概率
		// sprintf(buf, "%.2f", confidences[index]);
		// detectResult.probability = buf;

		// char buf2[30]; // 类名长度
		// strncpy(buf2, className_[idx].c_str(), sizeof(buf2) - 1); // 安全拷贝，防止溢出
		// buf2[sizeof(buf2) - 1] = '\0'; // 确保字符串以null终止
		// detectResult.className = buf2;
		
		
		//memcpy(&detectResult.className, &className_[idx], sizeof(className_[idx]));
		//detectResult.className = std::string(className_[idx]); // 拷贝构造

		//detectResult.className = className_[idx].c_str();
    	//detectResult.probability = std::string(cv::format("%.2f", confidences[index])).c_str();
		
        result->push_back(detectResult);
		std::cout << "detectResult.className: " << detectResult.className << std::endl;
		
	}

	// 显示 FPS
	float t = (cv::getTickCount() - startTick_) / static_cast<float>(cv::getTickFrequency());
	cv::putText(this->srcImg_, cv::format("FPS: %.2f", 1.0 / t),
				cv::Point(20, 40), cv::FONT_HERSHEY_PLAIN, 2.0, cv::Scalar(255, 0, 0), 2, 8);

	cv::imshow("OpenCV4.5.4 + YOLOv5", this->srcImg_);
    //return detectResultVector;

}

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-05-29, lium
* describe: 初始创建.
*************************************************************************/