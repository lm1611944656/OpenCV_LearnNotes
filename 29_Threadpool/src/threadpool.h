#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <opencv2/opencv.hpp>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "imgprocess.h"


class CThreadPool
{
public:
    explicit CThreadPool();
    ~CThreadPool();

public:
    /**设置线程池 */
    void setUp(int threadCount);

    /**提交要处理的任务 */
    int submitTask(const cv::Mat &img, int id);

    /**获取任务处理结果 */
    int getTargetResult(cv::Mat &resultImg, int id);

    /**停止所有线程 */
    void stopAll();

private:
    /**线程池的工作场 */
    void worker(int id);

private:
    /**线程池当前状态，默认是没有停止运行的。也就是stop = false*/
    bool stop;  

    /**处理任务的对象 */
    std::vector<std::shared_ptr<CImgProcess>> taskProcessorObj;

    /**线程对象 */
    std::vector<std::thread> threads;

    /**待处理的任务 */
    std::queue<std::pair<int, cv::Mat>> tasks;

    /**处理任务的对象去获取任务时需要一把锁 */
    std::mutex mtx1;

    /**处理任务的对象去获取任务时需要一把锁, 这把锁的条件变量 */
    std::condition_variable cv_task;

    /**处理对象，处理完成的结果 */
    std::map<int, cv::Mat> img_results;

    /**处理任务的对象，处理完成的结果需要放置到Map中，所以需要一把锁 */
    std::mutex mtx2;

    /**处理任务的对象，处理完成的结果需要放置到Map中，所以需要一把锁，这把锁自然要一个条件变量 */
    std::condition_variable cv_result;
};

#endif /**THREADPOOL_H */