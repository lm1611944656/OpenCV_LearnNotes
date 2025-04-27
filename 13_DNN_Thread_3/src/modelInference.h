/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: modelInference.h
*   软件模块: 应用模块
*   版 本 号: 1.0
*   生成日期: 2025-04-23
*   作    者: lium
*   功    能: 模型推理
* 
*************************************************************************/

#ifndef MODELINFERENCE_H
#define MODELINFERENCE_H

#include <thread>
#include "sharedQueue.h"

class Inference : public std::thread {
public:
    // 构造函数和析构函数
    Inference(SharedQueue<Job>& sharedQueue);
    ~Inference();

    /** 线程停止函数 */
    void threadStop(void);

private:
    /** 线程运行函数 */
    void run();

    /** 共享队列引用 */
    SharedQueue<Job>& m_sharedQueue;

    /** 当前线程是否正在运行 */
    volatile bool isRunning; 
};

#endif // INFERENCETHREAD_H


/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-04-23, lium
* describe: 初始创建.
*************************************************************************/