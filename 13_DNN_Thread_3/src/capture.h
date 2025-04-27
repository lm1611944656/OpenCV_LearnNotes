/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: capture.h
*   软件模块: 应用模块
*   版 本 号: 1.0
*   生成日期: 2025-04-23
*   作    者: lium
*   功    能: 视频捕获
* 
*************************************************************************/

#ifndef CAPTURE_H
#define CAPTURE_H

#include <thread>
#include <string>
#include "sharedQueue.h"

class Capture : public std::thread {
public:
    Capture(SharedQueue<Job>& sharedQueue, std::function<std::string(int)> taskFunc);
    ~Capture();

    /**线程停止函数 */
    void threadStop(void);

private:
    /**线程运行函数 */
    void run();

    /**当前线程是否正在执行 */
    volatile bool isCaptureRuning; // 使用 atomic 保证线程安全

    /**通过共享队列来传递数据 */
    SharedQueue<Job>& m_sharedQueue;

    /**线程累计执行了多少次 */
    int m_id;

    /**线程需要处理的函数 */
    std::function<std::string (int)> m_taskFunc;
};

#endif

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-04-23, lium
* describe: 初始创建.
*************************************************************************/