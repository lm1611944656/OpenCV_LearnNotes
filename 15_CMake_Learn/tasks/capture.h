/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: capture.h
*   软件模块: 应用模块
*   版 本 号: 1.0
*   生成日期: 2025-04-23
*   作    者: lium
*   功    能: 视频捕获
* 
*************************************************************************/

#ifndef CAPTURE_H
#define CAPTURE_H

#include <thread>
#include <functional>

class CCapture : public std::thread{
public:
    /**定义普通构造函数 */
    CCapture(std::function<void(int)> handleTask);
    ~CCapture();

    /**禁止拷贝构造和赋值操作符*/
    CCapture(const CCapture&) = delete;
    CCapture& operator=(const CCapture&) = delete;

    /**关闭线程 */
    void stopThread(void);

private:
    /**线程体 */
    void run(void);

    /**线程需要处理的函数 */
    std::function<void(int)> taskFunc;

private:
    /**是否关闭线程 */
    volatile bool isStopThread;

    int m_id;
};


#endif /**CAPTURE_H */

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-04-23, lium
* describe: 初始创建.
*************************************************************************/