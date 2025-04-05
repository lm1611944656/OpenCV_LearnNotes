#ifndef __MY_THREAD__
#define __MY_THREAD__


#include <string>

/** 打印几G的内容是一个耗时的工作 */
void readWord(const std::string &value, const int sleepTime);


#endif /** __MY_THREAD__ */
