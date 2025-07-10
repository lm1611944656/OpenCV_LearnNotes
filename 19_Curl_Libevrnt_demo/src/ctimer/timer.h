/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: timer.h
*   软件模块: 应用模块
*   版 本 号: 1.0
*   生成日期: 2025-06-11
*   作    者: lium
*   功    能: 用户自定义的定时器，最小是ms级别
* 
*************************************************************************/

#ifndef LIBEVENTTIMER_H
#define LIBEVENTTIMER_H

#include <event2/event.h>
#include <memory>


class LibeventTimer {
public:
    LibeventTimer();
    ~LibeventTimer();

public:
    /**定时器回调函数 */
    typedef void (*TimerCallback)(void);

    // 设置定时器间隔（毫秒），repeat为true表示重复触发，callback是定时器触发时调用的回调函数
    void setTimer(int milliseconds, bool repeat, TimerCallback callback);

    // 启动定时器
    void start();

    // 停止定时器
    void stop();

private:
    static void timerCb(evutil_socket_t fd, short event, void *arg);

    struct event_base *base_;
    struct event *timerEvent_;
    bool running_;
    TimerCallback callback_; // 用户提供的回调函数
};

#endif  /**LIBEVENTTIMER_H */

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-06-11, lium
* describe: 初始创建.
*************************************************************************/

