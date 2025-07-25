#include <iostream>

#if 0
#include "YoloV8Detect.h"
int main(int argc, char **argv)
{
    YoloV8Detect yolo_detect;
    yolo_detect.loadClassNames("weigths/yolov8s_coco80_20250608.txt");
	yolo_detect.loadModel("weigths/yolov8s_coco80_20250608.onnx");

	cv::Mat cv_src = cv::imread("data/test.jpg");
	// cv::VideoCapture cap("F20.mp4");

	std::vector<cv::Scalar> color;
	std::srand(time(0));//使用当前时间作为随机数种子
	for (int i = 0; i < 3; i++)
	{
		int b = rand() % 256;
		int g = rand() % 256;
		int r = rand() % 256;
		color.push_back(cv::Scalar(b, g, r));
	}

	yolo_detect.image_detect(cv_src, color);
    return 0;
}
#endif

#if 1
#include "objectDetect.h"
#include <memory>

int main(int argc, char **argv)
{
    std::string className = "weigths/yolov8s_coco80_20250608.txt";
    std::string modelPath = "weigths/yolov8s_coco80_20250608.onnx";
    std::string imgPath = "data/bus.jpg";
    std::shared_ptr<CObjectDetect> objDetect = std::make_shared<CObjectDetect>(modelPath, className);
    objDetect->modelDetect(imgPath);

    cv::waitKey(0);
    cv::destroyAllWindows();
}

#endif
