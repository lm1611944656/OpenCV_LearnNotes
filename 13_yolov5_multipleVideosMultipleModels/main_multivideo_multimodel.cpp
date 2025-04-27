#include <iostream>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
// #include <windows.h>

#include "yolov5.h"


bool stop = false;

void print_time(std::string video)
{
	auto now = std::chrono::system_clock::now();
	uint64_t dis_millseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count()
		- std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count() * 1000;
	time_t tt = std::chrono::system_clock::to_time_t(now);
	auto time_tm = localtime(&tt);
	char time[100] = { 0 };
	sprintf(time, "%d-%02d-%02d %02d:%02d:%02d %03d", time_tm->tm_year + 1900,
		time_tm->tm_mon + 1, time_tm->tm_mday, time_tm->tm_hour,
		time_tm->tm_min, time_tm->tm_sec, (int)dis_millseconds);
	std::cout << "infer " << video << " 当前时间为：" << time << std::endl;
}

void capture(std::string video, cv::dnn::Net net)
{
	cv::VideoCapture cap(video);
	while (cv::waitKey(1) < 0)
	{
		cv::Mat frame;
		cap.read(frame);

		if (frame.empty())
		{
			stop = true;
			break;
		}

		cv::Mat input_image = frame, blob, output_image;
		pre_process(input_image, blob);

		std::vector<cv::Mat> network_outputs;
		process(blob, net, network_outputs);

		post_process(input_image, output_image, network_outputs);

		print_time(video);
		cv::imshow(video, output_image);
	}
}

int main(int argc, char* argv[])
{
	std::string video1("test1.mp4");
	std::string video2("test2.mp4");

	cv::dnn::Net net1 = cv::dnn::readNet("yolov5n.onnx");
	cv::dnn::Net net2 = cv::dnn::readNet("yolov5s.onnx");

	std::thread t1(capture, video1, net1);
	std::thread t2(capture, video2, net2);

	t1.join();
	t2.join();

	return 0;
}