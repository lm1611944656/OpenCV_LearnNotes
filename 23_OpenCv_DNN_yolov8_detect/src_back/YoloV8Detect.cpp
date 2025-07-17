#include "YoloV8Detect.h"
#include <fstream>

void LetterBox(
    const cv::Mat& image, 
    cv::Mat& outImage, 
    cv::Vec4d& params, 
    const cv::Size& newShape,
	bool autoShape = false,
	bool scaleFill = false,
	bool scaleUp = true,
    int stride = 32, 
    const cv::Scalar& color = cv::Scalar(114, 114, 114))
{
	if (false) {
		int maxLen = MAX(image.rows, image.cols);
		outImage = cv::Mat::zeros(cv::Size(maxLen, maxLen), CV_8UC3);
		image.copyTo(outImage(cv::Rect(0, 0, image.cols, image.rows)));
		params[0] = 1;
		params[1] = 1;
		params[3] = 0;
		params[2] = 0;
	}

	cv::Size shape = image.size();
	float r = std::min((float)newShape.height / (float)shape.height,
		(float)newShape.width / (float)shape.width);
	if (!scaleUp)
		r = std::min(r, 1.0f);

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
	{
		cv::resize(image, outImage, cv::Size(new_un_pad[0], new_un_pad[1]));
	}
	else {
		outImage = image.clone();
	}

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

void DrawPred(cv::Mat& img, std::vector<OutputSeg> result, std::vector<std::string> classNames, std::vector<cv::Scalar> color, bool isVideo = false) 
{
	cv::Mat mask = img.clone();
	for (int i = 0; i < result.size(); i++) 
	{
		int left, top;
		left = result[i].box.x;
		top = result[i].box.y;
		int color_num = i;
		rectangle(img, result[i].box, color[result[i].id], 1.4, 8);
		if (result[i].boxMask.rows && result[i].boxMask.cols > 0)
			mask(result[i].box).setTo(color[result[i].id], result[i].boxMask);
		std::string label = classNames[result[i].id] + ":" + std::to_string(result[i].confidence);
		int baseLine;
		cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
		top = std::max(top, labelSize.height);
		//rectangle(frame, Point(left, top - int(1.5 * labelSize.height)), Point(left + int(1.5 * labelSize.width), top + baseLine), Scalar(0, 255, 0), FILLED);
		putText(img, label, cv::Point(left, top), cv::FONT_HERSHEY_SIMPLEX, 0.6, color[result[i].id], 2);
	}
	addWeighted(img, 0.5, mask, 0.5, 0, img); //add mask to src
	/*cv::namedWindow("1", 0);
	imshow("1", img);
	if (!isVideo)
		cv::waitKey();*/
	//destroyAllWindows();

}



YoloV8Detect::YoloV8Detect()
{
}

void YoloV8Detect::loadClassNames(std::string classNamesPath)
{
	std::ifstream fp(classNamesPath);
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
			this->_class_name.push_back(name);
	}
	fp.close();
    std::cout << "类别读取成功！" << std::endl;
}

void YoloV8Detect::loadModel(const std::string& modelPath)
{
    /**读取网络模型 */
    _net = cv::dnn::readNetFromONNX(modelPath);
    if (_net.empty()) {
        throw std::runtime_error("无法加载模型文件: " + modelPath);
    }
    std::cout << "模型加载成功！" << std::endl;
}

bool YoloV8Detect::detect(cv::Mat& cv_src,std::vector<OutputSeg>& output)
{
	cv::Mat blob;
	output.clear();
	int col = cv_src.cols;//w
	int row = cv_src.rows;//h
	cv::Mat net_input_img;
	cv::Vec4d params;
	LetterBox(cv_src, net_input_img, params, cv::Size(_net_width, _net_height));//调整图像大小
	cv::dnn::blobFromImage(net_input_img, blob, 1 / 255.0, cv::Size(_net_width, _net_height), cv::Scalar(0, 0, 0), true, false);

	_net.setInput(blob);
	std::vector<cv::Mat> net_output_img;

	_net.forward(net_output_img, _net.getUnconnectedOutLayersNames()); //get outputs

	std::vector<int> class_ids;// res-class_id
	std::vector<float> confidences;// res-conf 
	std::vector<cv::Rect> boxes;// res-box
	cv::Mat output0 = cv::Mat(cv::Size(net_output_img[0].size[2], net_output_img[0].size[1]), CV_32F, (float*)net_output_img[0].data).t();  //[bs,116,8400]=>[bs,8400,116]
	int net_width = output0.cols;
	int rows = output0.rows;
	float* pdata = (float*)output0.data;
	for (int r = 0; r < rows; ++r) {
		cv::Mat scores(1, _class_name.size(), CV_32FC1, pdata + 4);
		cv::Point class_id_point;
		double max_class_socre;
		minMaxLoc(scores, 0, &max_class_socre, 0, &class_id_point);
		max_class_socre = (float)max_class_socre;
		if (max_class_socre >= _class_threshold) {
			//rect [x,y,w,h]
			float x = (pdata[0] - params[2]) / params[0];
			float y = (pdata[1] - params[3]) / params[1];
			float w = pdata[2] / params[0];
			float h = pdata[3] / params[1];
			int left = MAX(int(x - 0.5 * w + 0.5), 0);
			int top = MAX(int(y - 0.5 * h + 0.5), 0);
			class_ids.push_back(class_id_point.x);
			confidences.push_back(max_class_socre);
			boxes.push_back(cv::Rect(left, top, int(w + 0.5), int(h + 0.5)));
		}
		pdata += net_width;//next line
	}
	//NMS
	std::vector<int> nms_result;
	cv::dnn::NMSBoxes(boxes, confidences, _class_threshold, _nms_threshold, nms_result);
	std::vector<std::vector<float>> temp_mask_proposals;
	cv::Rect holeImgRect(0, 0, cv_src.cols, cv_src.rows);
	for (int i = 0; i < nms_result.size(); ++i) {
		int idx = nms_result[i];
		OutputSeg result;
		result.id = class_ids[idx];
		result.confidence = confidences[idx];
		result.box = boxes[idx] & holeImgRect;
		output.push_back(result);
	}
	if (output.size())
		return true;
	else
		return false;
}

void YoloV8Detect::image_detect(cv::Mat &cv_src,std::vector<cv::Scalar> color) {
	if (cv_src.empty())
	{
		std::cout << "read image empty!" << std::endl;
	}
	std::vector<OutputSeg> result;
	detect(cv_src, result);
	DrawPred(cv_src, result, _class_name, color);
	cv::namedWindow("aod", 0);
	cv::imshow("aod", cv_src);
	cv::waitKey();
}

void YoloV8Detect::video_detect(cv::VideoCapture &cap, std::vector<cv::Scalar> color) {
	std::string output = "output.mp4";
	int fps = int(cap.get(5));
	cv::VideoWriter videoWriter;
	while (true) 
	{
		cv::Mat cv_src;
		cap.read(cv_src);
		if (cv_src.empty()) 
		{
			break;
		}
		std::vector<OutputSeg> result;
		detect(cv_src, result);
		DrawPred(cv_src, result, _class_name, color);

		if (!videoWriter.isOpened()) 
		{
			int fourcc=cv::VideoWriter::fourcc('m', 'p', '4', 'v');
			videoWriter.open(output, fourcc, fps, cv::Size(cv_src.cols, cv_src.rows));
		}
		videoWriter.write(cv_src);
		cv::namedWindow("aod", 0);
		cv::imshow("aod", cv_src);
		cv::waitKey(25);
		/*if(cv::getWindowProperty("aod", cv::WND_PROP_AUTOSIZE) < 1){
			break;
		}*/
	}
	cap.release();
	videoWriter.release();
	cv::destroyAllWindows();
}
