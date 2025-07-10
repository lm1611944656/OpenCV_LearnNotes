/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: timer.cpp
*   软件模块: 应用模块
*   版 本 号: 1.0
*   生成日期: 2025-06-11
*   作    者: lium
*   功    能: 用户自定义的定时器，最小是ms级别
* 
*************************************************************************/
#include "timer.h"

#include "timer.h"
#include <iostream>

LibeventTimer::LibeventTimer() 
: base_(nullptr), timerEvent_(nullptr), running_(false) 
{
    /**创建一个事件管理系统 */
    base_ = event_base_new();
    if (!base_) {
        std::cerr << "无法创建 event_base" << std::endl;
    }
}

LibeventTimer::~LibeventTimer() {
    if (running_) {
        this->stop();
    }
    if (timerEvent_) {
        event_free(timerEvent_);
    }
    if (base_) {
        event_base_free(base_);
    }
}

void LibeventTimer::setTimer(int milliseconds, bool repeat, TimerCallback callback) {
    callback_ = callback; // 保存用户提供的回调函数

    if (timerEvent_) {
        event_del(timerEvent_);
        event_free(timerEvent_);
    }

    timerEvent_ = event_new(base_, -1, repeat ? EV_PERSIST : 0, LibeventTimer::timerCb, this);
    if (!timerEvent_) {
        std::cerr << "无法创建 event" << std::endl;
        return;
    }

    struct timeval tv = {milliseconds / 1000, (milliseconds % 1000) * 1000};
    event_add(timerEvent_, &tv);
}

void LibeventTimer::start() {
    if (!base_) return;

    running_ = true;
    event_base_dispatch(base_);
}

void LibeventTimer::stop() {
    if (!base_ || !running_) return;

    running_ = false;
    event_base_loopexit(base_, nullptr);
}

void LibeventTimer::timerCb(evutil_socket_t fd, short event, void *arg) {
    LibeventTimer* self = static_cast<LibeventTimer*>(arg);
    if (self && self->callback_) {
        self->callback_(); // 调用用户的回调函数
    }
}

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-06-11, lium
* describe: 初始创建.
*************************************************************************/
