#include <iostream>
#include <thread>
#include "./src/utils/comment.h"
#include "./src/func1/my_thread.h"
#include "./src/func2/func2.h"


int main(int argc, char **argv){
    std::cout << "hello world!" << std::endl;

    /** utils.h */
    printLineFile();

    /** 创建一个线程 */
    std::thread child_thread1(readWord, "读取word文件中的内容", 5);

    /** 创建一个线程 */
    std::thread child_thread2(readWord, "读取PDF文件中的内容", 3);

    /**等待线程执行完毕 */
    child_thread1.join();
    child_thread2.join();

    /** 模型推理 */
    model_infer();
    return 0;
}