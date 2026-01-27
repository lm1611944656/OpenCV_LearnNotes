/************************************************************************
*
*   Copyright (C), 2012-2020, GTBS. Co., Ltd.
*
*   软件模块 ：基础框架
*   功   能	：线程安全的队列
*
*************************************************************************/
#ifndef __MQUEUE__
#define __MQUEUE__

#include <queue>
#include <condition_variable>

#include "config.h"

// 线程安全队列模板类
template <typename T>
class ThreadSafeQueue {
public:
	ThreadSafeQueue() = default;
	~ThreadSafeQueue() = default;

	// 禁止拷贝和移动（简化设计，也可按需实现）
	ThreadSafeQueue(const ThreadSafeQueue&) = delete;
	ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;
	ThreadSafeQueue(ThreadSafeQueue&&) = delete;
	ThreadSafeQueue& operator=(ThreadSafeQueue&&) = delete;

	// 入队（超时返回false，队列满时丢弃最旧元素）
	bool push(const T& data, int timeout_ms = QUEUE_TIMEOUT_MS) {
		std::unique_lock<std::mutex> lock(mtx_);

		// 队列满时丢弃最旧元素
		if (queue_.size() >= MAX_QUEUE_SIZE) {
			if (!queue_.empty()) {
				queue_.pop();
				dropped_count_++; // 关键：记录一次丢帧
			}
		}

		// 等待队列有空间（带超时）
		if (cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms),
			[this]() { return queue_.size() < MAX_QUEUE_SIZE; })) {
			queue_.push(data);
			cv_.notify_one();
			return true;
		}
		return false;
	}

	// 出队（超时返回false）
	bool pop(T& data, int timeout_ms = QUEUE_TIMEOUT_MS) {
		std::unique_lock<std::mutex> lock(mtx_);
		if (cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms),
			[this]() { return !queue_.empty(); })) {
			data = queue_.front();
			queue_.pop();
			return true;
		}
		return false;
	}

	// 清空队列
	void clear() {
		std::lock_guard<std::mutex> lock(mtx_);
		while (!queue_.empty()) {
			queue_.pop();
		}
	}

	// 获取队列大小
	size_t size() const {
		std::lock_guard<std::mutex> lock(mtx_);
		return queue_.size();
	}

	// 判断队列是否为空
	bool empty() const {
		std::lock_guard<std::mutex> lock(mtx_);
		return queue_.empty();
	}

	// 获取并重置丢帧计数
    size_t getAndResetDroppedCount() {
        return dropped_count_.exchange(0);
    }

	// 只读取当前丢帧数（不清零）
	size_t getDroppedCount() const {
        return dropped_count_.load();
    }

private:
	std::queue<T> queue_;
	mutable std::mutex mtx_;
	std::condition_variable cv_;

	std::atomic<size_t> dropped_count_{0}; // 累计丢帧数
};

#endif /**__MQUEUE__ */
/*************************************************************************
* 改动历史纪录：
Revision 1.0, 2026-01-20, lium
describe: 初始创建.
*************************************************************************/