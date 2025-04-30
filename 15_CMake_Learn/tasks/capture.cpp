/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: capture.cpp
*   软件模块: 应用模块
*   版 本 号: 1.0
*   生成日期: 2025-04-23
*   作    者: lium
*   功    能: 视频捕获
* 
*************************************************************************/
#include "capture.h"
#include <unistd.h>
#include <iostream>

CCapture::~CCapture(){
    /**如果当前线程还在运行 */
    if(this->joinable()){
        /**阻塞等待 */
        this->join();
    }
}

/**定义普通构造函数 */
CCapture::CCapture(std::function<void(int)> handleTask)
:std::thread(&CCapture::run, this),
isStopThread(true),
taskFunc(handleTask),
m_id(0)
{

}

void CCapture::run(void)
{
    while(isStopThread){
        taskFunc(m_id++);
        std::cout << "当前值为：" << m_id << std::endl;

        usleep(5000);
    }
}

void CCapture::stopThread(void){
    isStopThread = false;
}
/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-04-23, lium
* describe: 初始创建.
*************************************************************************/