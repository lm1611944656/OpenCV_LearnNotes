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
    SharedQueue sharedQueue(10);

    //Capture capture(sharedQueue, captureImg);
    std::unique_ptr<Capture> capture = std::make_unique<Capture>(sharedQueue, captureImg);
    Inference inference(sharedQueue);

    /**让主线程在此阻塞5000 毫秒后，再执行后面的代码 */
    usleep(5000 * 1000); // 休眠 5000 毫秒（注意单位是微秒）

    /**通知子线程停止 */
    inference.threadStop();
    capture->threadStop();

    /**等待子线程退出 */
    capture->join(); // 等待 capture 线程退出
    inference.join(); // 等待 inference 线程退出

    std::cout << "All threads stopped. Main thread exiting." << std::endl;

    return 0;
}