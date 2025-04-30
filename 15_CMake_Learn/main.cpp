#include <iostream>
#include <memory>
#include "capture.h"
#include "inference.h"

static void taskFunc(int){
    
}

int main(int argc, char **argv){
    std::unique_ptr<CCapture> m_capture  = std::make_unique<CCapture>(taskFunc);

    // 让程序运行一段时间
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // 停止 Capture2 线程
    m_capture->stopThread();
    m_capture->join();

    std::cout << "All threads have finished." << std::endl;

    inferenceTask();

    return 0;
}