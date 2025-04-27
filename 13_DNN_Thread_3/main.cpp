#include "capture.h"
#include "modelInference.h"
#include "sharedQueue.h"
#include <unistd.h> // 包含 usleep 函数
#include <iostream>
#include <memory>


std::string captureImg(int value){
    std::string name = std::to_string(value) + ".jpg";
    return name;
}

int main() {
    // 创建共享队列
    SharedQueue<Job> sharedQueue;

    // 创建 Capture 线程
    auto captureTask = [](int id) -> std::string {
        return "Job_" + std::to_string(id);
    };

    /**创建第一个捕获线程 */ 
    Capture capture(sharedQueue, captureTask);

    /**创建第二个捕获线程 */
    std::unique_ptr<Capture> capture2 = std::make_unique<Capture>(sharedQueue, captureImg);

    // 创建 Inference 线程
    Inference inference(sharedQueue);

    // 让程序运行一段时间
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // 停止 Capture2 线程
    capture2->threadStop();
    capture2->join();

    // 停止 Capture1 线程
    capture.threadStop();
    capture.join();

    // 关闭共享队列
    sharedQueue.shut_down();

    // 停止 Inference 线程
    inference.threadStop();
    inference.join();

    std::cout << "All threads have finished." << std::endl;
    return 0;
}