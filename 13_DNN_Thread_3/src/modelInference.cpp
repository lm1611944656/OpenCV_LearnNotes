/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: modelInference.cpp
*   软件模块: 应用模块
*   版 本 号: 1.0
*   生成日期: 2025-04-23
*   作    者: lium
*   功    能: 模型推理
* 
*************************************************************************/

#include "modelInference.h"
#include <iostream>
#include <unistd.h> // 包含 usleep 函数

// 构造函数
Inference::Inference(SharedQueue<Job>& sharedQueue)
    : std::thread(&Inference::run, this), m_sharedQueue(sharedQueue) {
    isRunning = true;
}

// 析构函数
Inference::~Inference() {
    if (joinable()) {
        join();
    }
}

// 线程运行函数
void Inference::run() {
    while (isRunning) {
        try {
            // 从共享队列中获取任务
            Job job = m_sharedQueue.pop();

            // 打印接收到的任务
            std::cout << "Inference received: " << job.input << std::endl;

            // 模拟推理处理
            std::string result = job.input + " after inference";

            // 设置异步结果
            if (job.pro) {
                job.pro->set_value(result);
            }

            // 模拟推理延迟
            usleep(500 * 1000); // 500 毫秒
        } catch (const std::runtime_error& e) {
            // 捕获队列关闭或空的异常
            if (m_sharedQueue.is_shutdown()) {
                std::cout << "Inference thread detected queue shutdown." << std::endl;
                break;
            } else {
                std::cerr << "Error in inference thread: " << e.what() << std::endl;
            }
        } catch (...) {
            // 捕获其他未知异常
            std::cerr << "Unknown error in inference thread." << std::endl;
        }
    }
    std::cout << "Inference Thread stopping..." << std::endl;
}

// 停止线程
void Inference::threadStop(void) {
    isRunning = false;
}

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-04-23, lium
* describe: 初始创建.
*************************************************************************/