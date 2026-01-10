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
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{

    // std::string className = "weigths/YoloV8l_water_20250717.txt";
    // std::string modelPath = "weigths/YoloV8l_water_20250717.onnx";
    // std::string imgPath = "data/test.jpg";
    // std::shared_ptr<CObjectDetect> objDetect = std::make_shared<CObjectDetect>(modelPath, className);
    // objDetect->modelDetect(imgPath);


    for (int i = 0; i < 50; i++) {
        std::string className = "weigths/YoloV8l_water_20250717.txt";
        std::string modelPath = "weigths/YoloV8l_water_20250717.onnx";
        std::string imgPath = "data/test.jpg";

        // 使用 new 创建对象
        CObjectDetect* objDetect = new CObjectDetect(modelPath, className);

        // 调用检测函数
        objDetect->modelDetect(imgPath);

        // 释放内存
        delete objDetect;
        objDetect = nullptr; // 避免悬空指针



        // 延时 1秒
        sleep(1); // Linux/macOS
    }
    cv::waitKey(0);
    cv::destroyAllWindows();
    return 0;

}

#endif

#if 0
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include "objectDetect.h"

std::string className = "weigths/YoloV8l_water_20250717.txt";
std::string modelPath = "weigths/YoloV8l_water_20250717.onnx";
std::string imgPath = "data/test.jpg";

// 假设你已经定义了这些
std::shared_ptr<CObjectDetect> objDetect = std::make_shared<CObjectDetect>(modelPath, className);

const int total_calls = 50;
const int num_threads = 4; // 使用4个线程

// 每个线程执行的任务函数
void worker(int start, int end, const std::string& imgPath) {
    for (int i = start; i < end; ++i) {
        std::cout << "Thread " << std::this_thread::get_id() 
                  << " executing detection " << i << std::endl;
        
        objDetect->modelDetect(imgPath);
        
        std::this_thread::sleep_for(std::chrono::seconds(2)); // sleep 2秒
    }
}

int main() {
    int calls_per_thread = total_calls / num_threads;
    int remainder = total_calls % num_threads;

    std::vector<std::thread> threads;

    int start = 0;
    for (int t = 0; t < num_threads; ++t) {
        int end = start + calls_per_thread + (t < remainder ? 1 : 0); // 处理余数
        threads.emplace_back(worker, start, end, imgPath);
        start = end;
    }

    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }

    std::cout << "All threads completed." << std::endl;
    return 0;
}
#endif