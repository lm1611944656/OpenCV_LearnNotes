#include "threadpool.h"
#include <iostream>
#include <chrono>

// 构造函数
CThreadPool::CThreadPool()
    : stop(false)
{
}

// 析构函数
CThreadPool::~CThreadPool()
{
    stopAll();
}

// 设置线程池大小和初始化
void CThreadPool::setUp(int threadCount)
{
    // 创建处理任务的对象
    for (int i = 0; i < threadCount; ++i) {
        std::shared_ptr<CImgProcess> taskProcessorObjItem = std::make_shared<CImgProcess>();
        taskProcessorObj.push_back(taskProcessorObjItem);
    }

    // 创建线程
    for (int i = 0; i < threadCount; ++i) {
        threads.emplace_back(&CThreadPool::worker, this, i); // 注意这里传递了 'this'
    }
}

// 工作线程执行的任务
void CThreadPool::worker(int id)
{
    while (!stop) {
        std::pair<int, cv::Mat> task;
        std::shared_ptr<CImgProcess> imgProcessObj = taskProcessorObj[id];

        {
            std::unique_lock<std::mutex> lock(mtx1);
            cv_task.wait(lock, [this] { return !tasks.empty() || stop; });

            if (stop) {
                return;
            }

            task = tasks.front();
            tasks.pop();
        }

        cv::Mat resultImg;
        imgProcessObj->run(task.second, resultImg);

        {
            std::lock_guard<std::mutex> lock(mtx2);
            // 生成文件名：data/result_123.jpg
            //std::string filename = "./data/result_" + std::to_string(task.first) + ".jpg";

            // 保存图像
            //bool success = cv::imwrite(filename, resultImg);

            img_results.insert({task.first, resultImg});
            cv_result.notify_one();
        }
    }
}

// 提交任务到线程池
int CThreadPool::submitTask(const cv::Mat &img, int id)
{
    {
        std::lock_guard<std::mutex> lock(mtx1);
        tasks.push({id, img});
    }
    cv_task.notify_one();

    return 0;
}

// 获取任务结果
int CThreadPool::getTargetResult(cv::Mat &resultImg, int id)
{
    int loop_cnt = 0;
    while (img_results.find(id) == img_results.end()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        loop_cnt++;

        if (loop_cnt > 1000) {
            std::cout << "getTargetImgResult timeout" << std::endl;
            return -1;
        }
    }

    std::lock_guard<std::mutex> lock(mtx2);
    resultImg = img_results[id];
    img_results.erase(id);

    return 0;
}

// 停止所有工作线程
void CThreadPool::stopAll()
{
    stop = true;
    cv_task.notify_all();
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}