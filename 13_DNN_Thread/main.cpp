#include <iostream>
#include <memory>  // std::shared_ptr
#include <future>  // std::promise
#include <queue>   // std::queue
#include <thread>
#include <unistd.h> // 包含 usleep 函数

/**线程之间的数据传递对象, 通过std::promise来实现 */
struct Job{
    std::string input;
    std::shared_ptr<std::promise<std::string>> pro; // 用于异步返回结果的 promise
};

/**线程之间的共享任务队列 */
std::queue<Job> jobs;
const int queueLimit = 5;

/**任务队列需要使用互斥锁，保存安全性 */
std::mutex lock;

/**线程之间需要有执行顺序，因此设置条件变量 */
std::condition_variable conditionVariables;

/**模拟“捕获”线程的行为， 生成任务并放入队列中 */
void capture(){
    /**累计捕获的帧数量 */
    int frameIDs = 0;

    while(true){

        /**创建一个数据传递对象 */
        Job job;
        {
            /**创建一个锁管理对象(创建的时候，就会加锁) */
            std::unique_lock<std::mutex> lockManagementObj(lock);

            /**捕获视频中的帧任务 */
            std::string frameName = std::to_string(frameIDs++) + ".jpg";
            std::cout << "capture: " << frameName << "队列中数据量：" << jobs.size() << std::endl;

            /**入队之前需要下判断队列中数据量 */
            /**等待队列中数据的量小于限制，在入队 */
            /**在等待的时候，就会释放锁。释放锁，目的是让消费者可以从队列中获取数据 */
            /**该断代码意思是：如果队列已满（jobs.size() >= limit），捕获线程会释放锁并进入等待状态，
             * 直到推理线程处理完任务后调用 cv.notify_all() 唤醒它 */
            conditionVariables.wait(lockManagementObj, [&](){
                return jobs.size() < queueLimit;
            });

            /**初始化任务传递对象 */
            job.input = frameName;
            job.pro = std::make_shared<std::promise<std::string>>();   // 创建一个新的promise对象
            jobs.push(job);                                 // 将任务加入队列
        }

        /**阻塞等待，等待推理线程返回结果 */
        auto result = job.pro->get_future().get();
        std::cout << result << std::endl;

        /**模拟捕获消耗的时间 */
        usleep(500);
    }
};

/**模拟“推理”线程，从队列中取出数据任务并且处理 */
void inference(){

    while(true){
        /**判断队列中是否有任务 */
        if(!jobs.empty()){

            {
                /**创建一个锁管理对象 */
                std::lock_guard<std::mutex> lockManagementObj(lock);
                
                /**从队列中取出第一个任务 */
                auto job = jobs.front();
                jobs.pop();

                /**由于队列中已经被取出任务了，有空的位置，所以可以唤醒其他线程的等待现象 */
                conditionVariables.notify_all();

                /**打印当前处理的任务消息 */
                std::cout << "inference: " << job.input << std::endl;

                /**推理捕获到的帧 */
                auto result = job.input + "   after inference";

                /**设置promise的值， 通知捕获线程任务已经完成 */
                job.pro->set_value(result);
            }
            usleep(1000);
        }
    }
};

int main(int argc, char **arhv){
    std::thread t0(capture);
    std::thread t1(inference);

    //等待线程结束(实际上他们是无限循环，不会结束)
    t0.join();
    t1.join();
    return 0;
}

/**
 * 捕获线程：
 *  1. 加锁；
 *  2. 去捕获视频帧，并且将帧放入到队列中；
 *  3. 释放锁；
 *  4. 放入之后就阻塞等待，等待推理线程返回推理结果；
 *  5. 一次依次循环1~5；
 * 
 *  推理线程：
 *  1. 加锁；
 *  2. 获取队列中的数据，进行模型推理；
 *  3. 将推理后的结果，放入到队列中；
 *  4. 释放锁；
 *  5. 依次循环1~5;
 * 
 * 注意细节：
 *  1. 捕获线程中使用的unique_lock锁， 相对于推理线程中的lock_guard锁更灵活；
 *  2. mutex是最底层的一种锁，实现对共享资源的保护；
 *  3. unique_lock是对mutex的一个封装，可以自己实现加锁，解锁，延时锁；可以配和condition_variable一起使用
 *  4. lock_guard是对mutex的一个封装，可以自己实现加锁，解锁，延时锁；可以配合condition_variable一起使用
 *  
 *  5. promise一般用于线程之间的数据传递。（一个线程设置值，一个线程获取值）
 *  6. condition_variable用于线程间的同步，协调线程的执行顺序；(一个线程等待条件，一个线程通知)
 *  7. condition_variable一定要和mutex配合使用；
 */