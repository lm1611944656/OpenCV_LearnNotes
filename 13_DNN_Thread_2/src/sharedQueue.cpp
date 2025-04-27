/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: sharedQueue.cpp
*   软件模块: 应用模块
*   版 本 号: 1.0
*   生成日期: 2025-04-23
*   作    者: lium
*   功    能: 共享队列(用于存储数据传输对象)
* 
*************************************************************************/

#include "sharedQueue.h"
#include <iostream>

SharedQueue::SharedQueue(const int &queueCapacity)
:m_limit(queueCapacity){

}

SharedQueue::~SharedQueue(){

}

// 向共享队列中添加一个任务（Job），并附带一个 std::promise 对象用于异步通信。
void SharedQueue::push(const std::string& input, std::shared_ptr<std::promise<std::string>> pro) {
    // 使用 std::unique_lock 锁定互斥量，确保线程安全。
    std::unique_lock<std::mutex> lock(m_mutex);

    // 等待条件变量 conditionVariable 满足条件：队列中的元素数量小于限制值 m_limit。
    // 如果条件不满足，则当前线程会阻塞在这里，直到其他线程通知条件变量。
    conditionVariable.wait(lock, [this]() { return m_queue.size() < m_limit; });

    // 创建一个新的 Job 对象，包含输入字符串和与之关联的 promise，并将其推入队列。
    m_queue.push(Job{input, pro});

    // 通知所有等待在条件变量上的线程，队列状态已经改变。
    conditionVariable.notify_all();
}

// 从共享队列中弹出一个任务（Job）。
bool SharedQueue::pop(Job& job) {
    // 使用 std::lock_guard 锁定互斥量，确保线程安全。
    std::lock_guard<std::mutex> lock(m_mutex);

    // 检查队列是否为空。如果为空，则返回 false 表示没有任务可以弹出。
    if (m_queue.empty()) {
        return false;
    }

    // 将队列头部的任务赋值给参数 job。
    job = m_queue.front();

    // 弹出队列头部的任务。
    m_queue.pop();

    // 通知所有等待在条件变量上的线程，队列状态已经改变。
    conditionVariable.notify_all();

    // 返回 true 表示成功弹出了一个任务。
    return true;
}

// 获取共享队列中当前的任务数量。
size_t SharedQueue::size() const {
    // 使用 std::lock_guard 锁定互斥量，确保线程安全。
    std::lock_guard<std::mutex> lock(m_mutex);

    // 返回队列的大小。
    return m_queue.size();
}

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-04-23, lium
* describe: 初始创建.
*************************************************************************/