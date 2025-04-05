#include <unistd.h>
#include <iostream>
#include "my_thread.h"
#include "../utils/comment.h"


void readWord(const std::string &value, const int sleepTime){
    /** 调用其他模块 */
    printLineFile();

    /** 延时一段时间，模拟读取数据比较耗时 */
    sleep(sleepTime);

    std::cout << value << std::endl;
}