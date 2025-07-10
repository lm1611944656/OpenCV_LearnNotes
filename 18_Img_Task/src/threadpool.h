/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: imgclassification.h
*   软件模块: 图像分类
*   版 本 号: 1.0
*   生成日期: 2025-05-29
*   作    者: lium
*   功    能: 图像分类头文件定义
* 
************************************************************************/

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

class ThreadPool
{
public:
    ThreadPool(size_t);
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;
    ~ThreadPool();
private:
    std::vector<std::thread> _threads;
    std::queue<std::function<void()>> _que;
    std::mutex _mutex;
    std::condition_variable _condition;
    bool _stop;
};

inline ThreadPool::ThreadPool(size_t num) : _stop(false)
{
    for(size_t i = 0;i<num;++i)
    {
        _threads.emplace_back(
                [this]
                {
                    for(;;)
                    {
                        std::function<void()> task;

                        {
                            std::unique_lock<std::mutex> lock(this->_mutex);
                            this->_condition.wait(lock, [this]{ return this->_stop || !this->_que.empty(); });
                            if(this->_stop && this->_que.empty())
                            {
                                return;
                            }

                            task = std::move(this->_que.front());
                            this->_que.pop();
                        }

                        task();
                    }
                }
        );
    }
}

template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
-> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(_mutex);

        if(_stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        _que.emplace([task](){ (*task)(); });
    }
    _condition.notify_one();
    return res;
}

inline ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _stop = true;
    }
    _condition.notify_all();
    for(std::thread &worker: _threads)
        worker.join();
}

#endif // THREAD_POOL_H

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-05-29, lium
* describe: 初始创建.
*************************************************************************/