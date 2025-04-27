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


Capture::Capture(SharedQueue& sharedQueue, std::function<std::string (int)> taskFunc)
: std::thread(&Capture::run, this), 
 m_sharedQueue(sharedQueue), 
 m_id(0),
 m_taskFunc(taskFunc)
{
    /**捕获构造函数 */
    isCaptureRuning = true;
}

Capture::~Capture() {
    if (joinable()) {
        join();
    }
}

void Capture::run() {
    while (isCaptureRuning) {
        /**需要捕获的对象 */
        std::string name = m_taskFunc(m_id++);
        std::cout << "capture: " << name << " jobs.size(): " << m_sharedQueue.size() << std::endl;

        /**入队 */
        auto pro = std::make_shared<std::promise<std::string>>();
        m_sharedQueue.push(name, pro);

        /**等待推理结果 */
        auto result = pro->get_future().get();
        std::cout << result << std::endl;

        usleep(500);
    }
    std::cout << "Capture Thread stoped..." << std::endl;
}

void Capture::threadStop(void){
    isCaptureRuning = false;
}

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-04-23, lium
* describe: 初始创建.
*************************************************************************/