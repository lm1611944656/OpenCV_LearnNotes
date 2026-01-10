#include <iostream>
#include <opencv2/opencv.hpp>
#include <memory>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include "imgprocess.h"
#include "threadpool.h"

// ================== 配置区 ==================
const int NUM_STREAMS = 3; // 改为你需要的路数，比如 10

// 示例 RTSP 地址（请替换为你的实际地址）
std::vector<std::string> rtsp_urls = {
    "rtsp://admin:bpg123456@10.10.12.228:554/h264/2/main/av_stream",
    "rtsp://admin:bpg123456@10.10.12.229:554/h264/2/main/av_stream",
    "rtsp://admin:bpg123456@10.10.12.230:554/h264/2/main/av_stream"
    // 添加更多...
};

// ================== 全局变量 ==================
static CThreadPool* yolov5_thread_pool = nullptr;

// 每路流是否已结束读取
static std::atomic<bool> stream_reading_finished[NUM_STREAMS]; // 全部默认 false

// ================== 工具函数 ==================
inline int make_task_id(int stream_id, int frame_index) {
    // 高8位：stream_id (0～255)，低24位：frame_index (0～16M)
    return (stream_id << 24) | (frame_index & 0x00FFFFFF);
}

inline int get_stream_id_from_task_id(int task_id) {
    return (task_id >> 24) & 0xFF;
}

inline int get_frame_index_from_task_id(int task_id) {
    return task_id & 0x00FFFFFF;
}

// ================== 生产者：读取单路视频流 ==================
void read_stream(int stream_id, const std::string& rtsp_addr) {
    cv::VideoCapture cap(rtsp_addr);
    if (!cap.isOpened()) {
        std::cerr << "[Stream " << stream_id << "] Failed to open RTSP stream!" << std::endl;
        stream_reading_finished[stream_id] = true;
        return;
    }

    std::cout << "[Stream " << stream_id << "] Opened successfully." << std::endl;
    int frame_index = 0;
    cv::Mat frame;

    while (cap.read(frame) && !frame.empty()) {
        int task_id = make_task_id(stream_id, frame_index++);
        yolov5_thread_pool->submitTask(frame.clone(), task_id);
    }

    cap.release();
    stream_reading_finished[stream_id] = true;
    std::cout << "[Stream " << stream_id << "] Finished reading." << std::endl;
}

// ================== 消费者：处理单路结果（保序！） ==================
void consume_stream_results(int stream_id, bool record) {
    cv::VideoWriter writer;
    if (record) {
        std::string out_file = "output_stream_" + std::to_string(stream_id) + ".mp4";
        writer.open(out_file, cv::VideoWriter::fourcc('a','v','c','1'), 25, cv::Size(1280, 720));
        if (!writer.isOpened()) {
            std::cerr << "[Stream " << stream_id << "] Failed to open VideoWriter!" << std::endl;
        }
    }

    int expected_frame = 0; // 本流期望的下一帧索引（从0开始，严格递增）
    int saved_count = 0;

    while (true) {
        cv::Mat result_img;
        int task_id = make_task_id(stream_id, expected_frame);

        // 等待本流的第 'expected_frame' 帧完成（线程池内部会等它处理完）
        int ret = yolov5_thread_pool->getTargetResult(result_img, task_id);

        if (ret == 0 && !result_img.empty()) {
            // 保存图像
            std::string filename = "./data/stream_" + std::to_string(stream_id) +
                                   "_frame_" + std::to_string(expected_frame) + ".jpg";
            cv::imwrite(filename, result_img);
            saved_count++;

            // 写入视频
            if (record && writer.isOpened()) {
                writer << result_img;
            }

            expected_frame++; // 下一帧
        } else {
            // 超时 or 获取失败
            // 检查是否该流已结束读取 且 所有帧都已处理完
            if (stream_reading_finished[stream_id]) {
                // 再等一小会儿，防止最后一帧还没提交
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                // 尝试最后一次
                if (yolov5_thread_pool->getTargetResult(result_img, task_id) == 0 && !result_img.empty()) {
                    std::string filename = "./data/stream_" + std::to_string(stream_id) +
                                           "_frame_" + std::to_string(expected_frame) + ".jpg";
                    cv::imwrite(filename, result_img);
                    if (record && writer.isOpened()) writer << result_img;
                    saved_count++;
                }
                break; // 确认结束
            }

            // 如果还没结束，短暂等待后重试
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    if (writer.isOpened()) writer.release();
    std::cout << "[Stream " << stream_id << "] Saved " << saved_count << " frames." << std::endl;
}

// ================== 主函数 ==================
int main(int argc, char** argv) {
    // 创建输出目录
    system("mkdir -p data");

    // 初始化线程池（线程数 >= NUM_STREAMS，建议多一些）
    yolov5_thread_pool = new CThreadPool();
    yolov5_thread_pool->setUp(NUM_STREAMS + 4); // 例如 3 路就开 7 个线程

    // 启动所有生产者和消费者
    std::vector<std::thread> readers;
    std::vector<std::thread> consumers;

    for (int i = 0; i < NUM_STREAMS; ++i) {
        readers.emplace_back(read_stream, i, rtsp_urls[i]);
        consumers.emplace_back(consume_stream_results, i, true);
    }

    // 等待所有线程结束
    for (auto& t : readers) t.join();
    for (auto& t : consumers) t.join();

    // 清理
    yolov5_thread_pool->stopAll();
    delete yolov5_thread_pool;
    cv::destroyAllWindows();

    std::cout << "All streams processed successfully!" << std::endl;
    return 0;
}