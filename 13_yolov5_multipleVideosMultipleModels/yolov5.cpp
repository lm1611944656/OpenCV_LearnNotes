#include "yolov5.h"


void LetterBox(const cv::Mat& image, cv::Mat& outImage, cv::Vec4d& params, const cv::Size& newShape,
	bool autoShape, bool scaleFill, bool scaleUp, int stride, const cv::Scalar& color)
{
	cv::Size shape = image.size();
	float r = std::min((float)newShape.height / (float)shape.height, (float)newShape.width / (float)shape.width);
	if (!scaleUp)
	{
		r = std::min(r, 1.0f);
	}

	float ratio[2]{ r, r };
	int new_un_pad[2] = { (int)std::round((float)shape.width * r),(int)std::round((float)shape.height * r) };

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


void pre_process(cv::Mat& image, cv::Mat& blob)
{
#ifdef  LETTERBOX
	cv::Vec4d params;
	cv::Mat letterbox;
	LetterBox(image, letterbox, params, cv::Size(INPUT_WIDTH, INPUT_HEIGHT));
	cv::dnn::blobFromImage(letterbox, blob, 1. / 255., cv::Size(INPUT_WIDTH, INPUT_HEIGHT), cv::Scalar(), true, false);
#else
	cv::dnn::blobFromImage(image, blob, 1. / 255., cv::Size(INPUT_WIDTH, INPUT_HEIGHT), cv::Scalar(), true, false);

#endif //  LETTERBOX
}

void process(cv::Mat& blob, cv::dnn::Net& net, std::vector<cv::Mat>& outputs)
{
	net.setInput(blob);
	net.forward(outputs, net.getUnconnectedOutLayersNames());
}

void scale_boxes(cv::Rect& box, cv::Size size)
{
#ifdef  LETTERBOX
	float gain = std::min(INPUT_WIDTH * 1.0 / size.width, INPUT_HEIGHT * 1.0 / size.height);
	int pad_w = (INPUT_WIDTH - size.width * gain) / 2;
	int pad_h = (INPUT_HEIGHT - size.height * gain) / 2;
	box.x -= pad_w;
	box.y -= pad_h;
	box.x /= gain;
	box.y /= gain;
	box.width /= gain;
	box.height /= gain;
#else
	box.x /= (INPUT_WIDTH * 1.0 / size.width);
	box.y /= (INPUT_HEIGHT * 1.0 / size.height);
	box.width /= (INPUT_WIDTH * 1.0 / size.width);
	box.height /= (INPUT_HEIGHT * 1.0 / size.height);
#endif //  LETTERBOX
}

void draw_result(cv::Mat& image, std::string label, cv::Rect box)
{
	cv::rectangle(image, box, cv::Scalar(0, 0, 255), 2);
	int baseLine;
	cv::Size label_size = cv::getTextSize(label, 1, 1, 1, &baseLine);
	cv::Point tlc = cv::Point(box.x, box.y);
	cv::Point brc = cv::Point(box.x, box.y + label_size.height + baseLine);
	cv::putText(image, label, cv::Point(box.x, box.y), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 1);
}

void post_process(cv::Mat& input_image, cv::Mat& output_image, std::vector<cv::Mat>& network_outputs)
{
	output_image = input_image.clone();

	std::vector<int> class_ids;
	std::vector<float> confidences;
	std::vector<cv::Rect> boxes;

	float* data = (float*)network_outputs[0].data;

	const int dimensions = 85;  
	const int rows = 3 * (INPUT_WIDTH / 8 * INPUT_HEIGHT / 8 + INPUT_WIDTH / 16 * INPUT_HEIGHT / 16 + INPUT_WIDTH / 32 * INPUT_HEIGHT / 32);
	for (int i = 0; i < rows; ++i)
	{
		float confidence = data[4];
		if (confidence >= CONFIDENCE_THRESHOLD)
		{
			float* classes_scores = data + 5;
			cv::Mat scores(1, class_names.size(), CV_32FC1, classes_scores);
			cv::Point class_id;
			double max_class_score;
			cv::minMaxLoc(scores, 0, &max_class_score, 0, &class_id);
			if (max_class_score > SCORE_THRESHOLD)
			{
				float x = data[0];
				float y = data[1];
				float w = data[2];
				float h = data[3];
				int left = int(x - 0.5 * w);
				int top = int(y - 0.5 * h);
				int width = int(w);
				int height = int(h);
				cv::Rect box = cv::Rect(left, top, width, height);
				scale_boxes(box, input_image.size());
				boxes.push_back(box);
				confidences.push_back(confidence);
				class_ids.push_back(class_id.x);
			}
		}
		data += dimensions;
	}

	std::vector<int> indices;
	cv::dnn::NMSBoxes(boxes, confidences, SCORE_THRESHOLD, NMS_THRESHOLD, indices);
	for (int i = 0; i < indices.size(); i++)
	{
		int idx = indices[i];
		cv::Rect box = boxes[idx];
		std::string label = class_names[class_ids[idx]] + ":" + cv::format("%.2f", confidences[idx]);
		draw_result(output_image, label, box);
	}
}

void post_process(cv::Mat& input_image, std::vector<cv::Rect>& output_boxes, std::vector<cv::Mat>& network_outputs)
{
	std::vector<int> class_ids;
	std::vector<float> confidences;
	std::vector<cv::Rect> boxes;

	float* data = (float*)network_outputs[0].data;

	const int dimensions = 85;
	const int rows = 3 * (INPUT_WIDTH / 8 * INPUT_HEIGHT / 8 + INPUT_WIDTH / 16 * INPUT_HEIGHT / 16 + INPUT_WIDTH / 32 * INPUT_HEIGHT / 32);
	for (int i = 0; i < rows; ++i)
	{
		float confidence = data[4];
		if (confidence >= CONFIDENCE_THRESHOLD)
		{
			float* classes_scores = data + 5;
			cv::Mat scores(1, class_names.size(), CV_32FC1, classes_scores);
			cv::Point class_id;
			double max_class_score;
			cv::minMaxLoc(scores, 0, &max_class_score, 0, &class_id);
			if (max_class_score > SCORE_THRESHOLD)
			{
				float x = data[0];
				float y = data[1];
				float w = data[2];
				float h = data[3];
				int left = int(x - 0.5 * w);
				int top = int(y - 0.5 * h);
				int width = int(w);
				int height = int(h);
				cv::Rect box = cv::Rect(left, top, width, height);
				scale_boxes(box, input_image.size());
				boxes.push_back(box);
				confidences.push_back(confidence);
				class_ids.push_back(class_id.x);
			}
		}
		data += dimensions;
	}

	std::vector<int> indices;
	cv::dnn::NMSBoxes(boxes, confidences, SCORE_THRESHOLD, NMS_THRESHOLD, indices);
	for (int i = 0; i < indices.size(); i++)
	{
		int idx = indices[i];
		cv::Rect box = boxes[idx];
		output_boxes.push_back(box);
	}
}
