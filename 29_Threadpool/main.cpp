#include <iostream>
#include <opencv2/opencv.hpp>
#include <memory>
#include "imgprocess.h"
#include "threadpool.h"


// 设置 RTSP 流地址
std::string rtsp_url1 = "rtsp://admin:bpg123456@10.10.12.228:554/h264/2/main/av_stream";

static int g_frame_start_id = 0; // 读取视频帧的索引
static int g_frame_end_id = 0;   // 模型处理完的索引

// 创建线程池
static CThreadPool *yolov5_thread_pool = nullptr;
bool end = false;

/**获取视频流(生产者) */
void read_stream(std::string rtsp_addr);

/**获取处理结果(消费者) */
void get_results(bool record = false);


int main(int argc, char **argv){

    // 实例化线程池
    yolov5_thread_pool = new CThreadPool();
    yolov5_thread_pool->setUp(6);

    // 读取视频
    std::thread read_stream_thread(read_stream, rtsp_url1);

    // 启动结果线程
    std::thread result_thread(get_results, true);

    // 等待线程结束
    read_stream_thread.join();
    result_thread.join();

	// 关闭窗口
    cv::destroyWindow("RTSP Stream");
    return 0;
}


void read_stream(std::string rtsp_addr)
{
    // 读取视频
    cv::VideoCapture cap(rtsp_addr);
    if (!cap.isOpened())
    {
        std::cout << "Failed to open video file: " << rtsp_addr << std::endl;
    }

    // 获取视频尺寸、帧率
    int width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    int fps = cap.get(cv::CAP_PROP_FPS);
    printf("Video size: %d x %d, fps: %d", width, height, fps);

    // 画面
    cv::Mat img;

    while (true)
    {

        // 读取视频帧
        cap >> img;
        if (img.empty())
        {
            printf("Video end.");
            // 等待一下没有处理结束的画面
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
            end = true;
            break;
        }

        // 提交任务，这里使用clone，因为不这样数据在内存中可能不连续，导致绘制错误
        yolov5_thread_pool->submitTask(img.clone(), g_frame_start_id++);
		std::cout << "读取到的视频尺寸为：" << img.size() << std::endl;
    }
    // 释放资源
    cap.release();
}

void get_results(bool record)
{
    // 记录开始时间
    auto start_all = std::chrono::high_resolution_clock::now();
    int frame_count = 0;
    std::string fps_str;
    std::string duration_str;

    cv::VideoWriter writer;
    if(record)
    {
        writer = cv::VideoWriter("thread_pool_demo.mp4", cv::VideoWriter::fourcc('a', 'v', 'c', '1'), 30, cv::Size(1280, 720));
    }
    // 开始计时
    auto start_1 = std::chrono::high_resolution_clock::now();

    while (true)
    {
        // 结果
        cv::Mat img;
        auto ret = yolov5_thread_pool->getTargetResult(img, g_frame_end_id++);

		// 生成文件名：data/result_123.jpg
		std::string filename = "./data/result_" + std::to_string(g_frame_end_id) + ".jpg";

		// 保存图像
		cv::imwrite(filename, img);

        // 如果读取完毕，且模型处理完毕，结束
        if (end && ret != 0)
        {
            break;
        }
        frame_count++;
        // all end
        auto end_all = std::chrono::high_resolution_clock::now();
        auto elapsed_all_2 = std::chrono::duration_cast<std::chrono::microseconds>(end_all - start_all).count() / 1000.f;
        // 每隔1秒打印一次
        if (elapsed_all_2 > 1000)
        {
            printf("Method2 Time:%fms, FPS:%f, Frame Count:%d", elapsed_all_2, frame_count / (elapsed_all_2 / 1000.0f), frame_count);
            fps_str = std::to_string(frame_count) + "fps";
            frame_count = 0;
            start_all = std::chrono::high_resolution_clock::now();
        }
        
        if(record)
        {
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_all - start_1).count() / 1000.f;
            duration_str = std::to_string(duration) + "ms";
            cv::putText(img, fps_str , cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 2);
            cv::putText(img, duration_str, cv::Point(10, 50), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0xFF, 0xFF, 0xFF), 2);
            // 写入视频帧
            writer << img;
			std::cout << "写入视频成功" << std::endl;
        }

    }
    // 结束所有线程
    yolov5_thread_pool->stopAll();
    if (writer.isOpened())
    {
        writer.release();
    }
    printf("Get results end.");
}