/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: capture.cpp
*   软件模块: 应用模块
*   版 本 号: 1.0
*   生成日期: 2025-04-23
*   作    者: lium
*   功    能: 视频捕获
* 
*************************************************************************/

#include "capture.h"
#include <iostream>
#include <memory>
#include <unistd.h> // 包含 usleep 函数


// 构造函数
Capture::Capture(SharedQueue<Job>& sharedQueue, std::function<std::string(int)> taskFunc)
    : std::thread(&Capture::run, this), 
      m_sharedQueue(sharedQueue), 
      m_id(0),
      m_taskFunc(taskFunc) {
    isCaptureRuning = true;
}

// 析构函数
Capture::~Capture() {
    if (joinable()) {
        join();
    }
}

void Capture::run() {
    while (isCaptureRuning) {
        // 捕获数据
        std::string name = m_taskFunc(m_id++);
        std::cout << "Capture: " << name << ", jobs.size(): " << m_sharedQueue.size() << std::endl;

        // 创建异步结果对象
        auto pro = std::make_shared<std::promise<std::string>>();

        // 封装为 Job 对象
        Job job;
        job.input = name;
        job.pro = pro;

        // 将任务入队
        m_sharedQueue.push_back(job);

        // 等待推理结果
        auto result = pro->get_future().get();
        std::cout << "Processed Result: " << result << std::endl;
        printf("********************************\r\n");

        // 模拟捕获延迟
        usleep(500 * 1000); // 500 毫秒
    }
    std::cout << "Capture Thread stopped..." << std::endl;
}

void Capture::threadStop(void){
    isCaptureRuning = false;
}

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-04-23, lium
* describe: 初始创建.
*************************************************************************/