/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: sharedQueue.h
*   软件模块: 应用模块
*   版 本 号: 1.0
*   生成日期: 2025-04-23
*   作    者: lium
*   功    能: 共享队列(用于存储数据传输对象)
* 
*************************************************************************/

#ifndef SHAREDQUEUE_H
#define SHAREDQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>       // std::shared_ptr
#include <string>
#include <future>  // std::promise

/**数据传递对象 */
struct Job {
    std::string input;
    std::shared_ptr<std::promise<std::string>> pro;
};


class SharedQueue {

public:
    explicit SharedQueue(const int &queuecapacity);
    ~SharedQueue();

public:
    void push(const std::string& input, std::shared_ptr<std::promise<std::string>> pro);
    bool pop(Job& job);
    size_t size() const;

private:
    std::queue<Job> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable conditionVariable;
    const int m_limit;
};

#endif /**SHAREDQUEUE_H */

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-04-23, lium
* describe: 初始创建.
*************************************************************************/