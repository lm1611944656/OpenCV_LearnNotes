#pragma once

#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>

//#define LETTERBOX

//常量
const int INPUT_WIDTH = 640;
const int INPUT_HEIGHT = 640;
const float SCORE_THRESHOLD = 0.5;
const float NMS_THRESHOLD = 0.5;
const float CONFIDENCE_THRESHOLD = 0.5;
const std::vector<std::string> class_names = {
	"person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
	"fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
	"elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
	"skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard",
	"tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
	"sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
	"potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone",
	"microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
	"hair drier", "toothbrush" };


//LetterBox处理
void LetterBox(const cv::Mat& image, cv::Mat& outImage,
	cv::Vec4d& params, //[ratio_x,ratio_y,dw,dh]
	const cv::Size& newShape = cv::Size(640, 640),
	bool autoShape = false,
	bool scaleFill = false,
	bool scaleUp = true,
	int stride = 32,
	const cv::Scalar& color = cv::Scalar(114, 114, 114));

//预处理
void pre_process(cv::Mat& image, cv::Mat& blob);

//网络推理
void process(cv::Mat& blob, cv::dnn::Net& net, std::vector<cv::Mat>& outputs);

//box缩放到原图尺寸
void scale_boxes(cv::Rect& box, cv::Size size);

//可视化函数
void draw_result(cv::Mat& image, std::string label, cv::Rect box);

//后处理
void post_process(cv::Mat& input_image, cv::Mat& output_image, std::vector<cv::Mat>& network_outputs);

void post_process(cv::Mat& input_image, std::vector<cv::Rect>& output_boxes, std::vector<cv::Mat>& network_outputs);
