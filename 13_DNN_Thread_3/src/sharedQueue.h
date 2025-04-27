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

#include <deque>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <future>  // std::promise

/**数据传递对象 */
struct Job {
    std::string input;
    std::shared_ptr<std::promise<std::string>> pro;
};

template <typename T>
class SharedQueue {
public:
    // 构造函数和析构函数
    SharedQueue();
    ~SharedQueue();

    // 获取队列头部元素（引用）
    T &front();

    // 弹出并返回队列头部元素
    T pop();

    // 弹出队列头部元素
    void pop_front();

    // 向队列尾部添加元素（支持左值和右值）
    void push_back(const T &item);
    void push_back(T &&item);

    // 关闭队列（清空队列并设置关闭标志）
    void shut_down();

    // 返回队列大小
    int size();

    // 判断队列是否为空
    bool empty();

    // 判断队列是否已关闭
    bool is_shutdown();

    // 通过索引访问队列元素（只读）
    T operator[](int k) {
        std::unique_lock<std::mutex> mlock(mutex_);
        return queue_[k];
    }

private:
    std::deque<T> queue_;               // 内部存储队列
    std::mutex mutex_;                  // 互斥锁，用于线程安全
    std::condition_variable cond_;      // 条件变量，用于线程同步
    bool shutdown = false;              // 队列关闭标志
};

// 构造函数
template <typename T>
SharedQueue<T>::SharedQueue() {}

// 析构函数
template <typename T>
SharedQueue<T>::~SharedQueue() {}

// 判断队列是否已关闭
template <typename T>
bool SharedQueue<T>::is_shutdown() {
    std::unique_lock<std::mutex> mlock(mutex_);
    return shutdown;
}

// 关闭队列
template <typename T>
void SharedQueue<T>::shut_down() {
    std::unique_lock<std::mutex> mlock(mutex_);
    queue_.clear();  // 清空队列
    shutdown = true; // 设置关闭标志
    cond_.notify_all(); // 唤醒所有等待的线程
}

// 弹出并返回队列头部元素
template <typename T>
T SharedQueue<T>::pop() {
    std::unique_lock<std::mutex> mlock(mutex_);
    cond_.wait(mlock, [this]() { return !queue_.empty() || shutdown; });

    if (shutdown && queue_.empty()) {
        throw std::runtime_error("Queue is shut down and empty.");
    }

    T rc(std::move(queue_.front()));
    queue_.pop_front();
    return rc;
}

// 获取队列头部元素（引用）
template <typename T>
T &SharedQueue<T>::front() {
    std::unique_lock<std::mutex> mlock(mutex_);
    cond_.wait(mlock, [this]() { return !queue_.empty() || shutdown; });

    if (shutdown && queue_.empty()) {
        throw std::runtime_error("Queue is shut down and empty.");
    }

    return queue_.front();
}

// 弹出队列头部元素
template <typename T>
void SharedQueue<T>::pop_front() {
    std::unique_lock<std::mutex> mlock(mutex_);
    cond_.wait(mlock, [this]() { return !queue_.empty() || shutdown; });

    if (!queue_.empty()) {
        queue_.pop_front();
    }
}

// 向队列尾部添加元素（支持左值）
template <typename T>
void SharedQueue<T>::push_back(const T &item) {
    std::unique_lock<std::mutex> mlock(mutex_);
    queue_.push_back(item);
    mlock.unlock();     // 提前解锁以减少锁的竞争
    cond_.notify_one(); // 唤醒一个等待的线程
}

// 向队列尾部添加元素（支持右值）
template <typename T>
void SharedQueue<T>::push_back(T &&item) {
    std::unique_lock<std::mutex> mlock(mutex_);
    queue_.push_back(std::move(item));
    mlock.unlock();     // 提前解锁以减少锁的竞争
    cond_.notify_one(); // 唤醒一个等待的线程
}

// 返回队列大小
template <typename T>
int SharedQueue<T>::size() {
    std::unique_lock<std::mutex> mlock(mutex_);
    int size = queue_.size();
    mlock.unlock();
    return size;
}

// 判断队列是否为空
template <typename T>
bool SharedQueue<T>::empty() {
    std::unique_lock<std::mutex> mlock(mutex_);
    bool is_empty = queue_.empty();
    mlock.unlock();
    return is_empty;
}

#endif /**SHAREDQUEUE_H */

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-04-23, lium
* describe: 初始创建.
*************************************************************************/