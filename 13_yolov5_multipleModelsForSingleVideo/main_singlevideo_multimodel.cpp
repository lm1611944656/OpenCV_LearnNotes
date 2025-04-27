#include <iostream>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <opencv2/opencv.hpp>

#include "yolov5.h"

struct Job {
    cv::Mat image;
    std::shared_ptr<std::promise<std::vector<cv::Rect>>> boxes;
};

class CaptureThread {
public:
    CaptureThread(cv::VideoCapture& cap, 
                  std::queue<Job>& jobs1, std::mutex& lock1, std::condition_variable& cv1,
                  std::queue<Job>& jobs2, std::mutex& lock2, std::condition_variable& cv2, 
                  int limit)
        : cap_(cap), 
          jobs1_(jobs1), lock1_(lock1), cv1_(cv1),
          jobs2_(jobs2), lock2_(lock2), cv2_(cv2),
          limit_(limit) {}

    void start() {
        while (cv::waitKey(1) < 0) {
            cv::Mat frame;
            cap_.read(frame);
            if (frame.empty()) {
                stop_ = true;
                break;
            }

            // 创建任务并加入队列
            {
                Job job1, job2;
                job1.image = frame.clone();
                job1.boxes = std::make_shared<std::promise<std::vector<cv::Rect>>>();
                job2.image = frame.clone();
                job2.boxes = std::make_shared<std::promise<std::vector<cv::Rect>>>();

                // 加入队列1
                {
                    std::unique_lock<std::mutex> lock(lock1_);
                    cv1_.wait(lock, [&]() { return jobs1_.size() < limit_; });
                    jobs1_.push(job1);
                }

                // 加入队列2
                {
                    std::unique_lock<std::mutex> lock(lock2_);
                    cv2_.wait(lock, [&]() { return jobs2_.size() < limit_; });
                    jobs2_.push(job2);
                }

                // 等待推理结果
                auto boxes1 = job1.boxes->get_future().get();
                auto boxes2 = job2.boxes->get_future().get();

                // 显示推理结果
                cv::Mat result1 = frame.clone(), result2 = frame.clone();
                for (const auto& box : boxes1) {
                    cv::rectangle(result1, box, cv::Scalar(255, 0, 0), 2); // 推理1用蓝色框
                }
                for (const auto& box : boxes2) {
                    cv::rectangle(result2, box, cv::Scalar(0, 0, 255), 2); // 推理2用红色框
                }

                cv::imshow("Result from Infer1", result1);
                cv::imshow("Result from Infer2", result2);
            }
        }
    }

    bool isStopped() const { return stop_; }

private:
    cv::VideoCapture& cap_;
    std::queue<Job>& jobs1_, &jobs2_;
    std::mutex& lock1_, &lock2_;
    std::condition_variable& cv1_, &cv2_;
    int limit_;
    bool stop_ = false;
};

class InferThread {
public:
    InferThread(std::queue<Job>& jobs, std::mutex& lock, std::condition_variable& cv, cv::dnn::Net net, int camera_id)
        : jobs_(jobs), lock_(lock), cv_(cv), net_(net), camera_id_(camera_id) {}

    void start() {
        while (true) {
            if (stop_) break;

            if (!jobs_.empty()) {
                Job job;
                {
                    std::lock_guard<std::mutex> lock(lock_);
                    job = jobs_.front();
                    jobs_.pop();
                    cv_.notify_all();
                }

                cv::Mat blob;
                pre_process(job.image, blob);

                std::vector<cv::Mat> network_outputs;
                process(blob, net_, network_outputs);

                std::vector<cv::Rect> boxes;
                post_process(job.image, boxes, network_outputs);

                job.boxes->set_value(boxes);
                print_time(camera_id_);
            }
            std::this_thread::yield();
        }
    }

    void stop() { stop_ = true; }

private:
    std::queue<Job>& jobs_;
    std::mutex& lock_;
    std::condition_variable& cv_;
    cv::dnn::Net net_;
    int camera_id_;
    bool stop_ = false;

    void print_time(int camera_id) {
        auto now = std::chrono::system_clock::now();
        uint64_t dis_millseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count()
            - std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count() * 1000;
        time_t tt = std::chrono::system_clock::to_time_t(now);
        auto time_tm = localtime(&tt);
        char time[100] = {0};
        sprintf(time, "%d-%02d-%02d %02d:%02d:%02d %03d", time_tm->tm_year + 1900,
                time_tm->tm_mon + 1, time_tm->tm_mday, time_tm->tm_hour,
                time_tm->tm_min, time_tm->tm_sec, (int)dis_millseconds);
        std::cout << "Infer " << camera_id << " 当前时间为：" << time << std::endl;
    }
};

int main(int argc, char* argv[]) {
    cv::VideoCapture cap("test2.mp4");

    std::queue<Job> jobs1, jobs2;
    std::mutex lock1, lock2;
    std::condition_variable cv1, cv2;
    const int limit = 10;

    cv::dnn::Net net1 = cv::dnn::readNet("yolov5n.onnx");
    cv::dnn::Net net2 = cv::dnn::readNet("yolov5s.onnx");

    CaptureThread capture(cap, jobs1, lock1, cv1, jobs2, lock2, cv2, limit);
    InferThread infer1(jobs1, lock1, cv1, net1, 1);
    InferThread infer2(jobs2, lock2, cv2, net2, 2);

    std::thread t0([&]() { capture.start(); });
    std::thread t1([&]() { infer1.start(); });
    std::thread t2([&]() { infer2.start(); });

    t0.join();
    infer1.stop();
    infer2.stop();
    t1.join();
    t2.join();

    return 0;
}