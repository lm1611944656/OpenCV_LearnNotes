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

Inference::Inference(SharedQueue& sharedQueue)
: std::thread(&Inference::run, this), m_sharedQueue(sharedQueue) 
{
    isRuning = true;
}

Inference::~Inference() {
    if (joinable()) {
        join();
    }
}

void Inference::run() {
    while (isRuning) {
        Job job;
        if (m_sharedQueue.pop(job)) {
            std::cout << "infer: " << job.input << std::endl;

            std::string result = job.input + " after infer";
            job.pro->set_value(result);

            usleep(1000);
        }
    }
    std::cout << "Inference Thread stopping..." << std::endl;
}

void Inference::threadStop(void){
    isRuning = false;
}

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-04-23, lium
* describe: 初始创建.
*************************************************************************/