/*************************************************************************
 *
 *   Copyright (C), 2017-2027, BPG. Co., Ltd.
 *
 *   文件名称: ccurl.h
 *   软件模块: 应用模块
 *   版 本 号: 1.0
 *   生成日期: 2025-06-11
 *   作    者: lium
 *   功    能: CURL
 *
 *************************************************************************/
#ifndef CCURL_H
#define CCURL_H


#include <string>

class CCurl
{
public:
    CCurl();
    ~CCurl();

    void test(); // 测试方法

private:
    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp);

    static void initCurl(); // 初始化 CURL 库（如果需要）
};

#endif /**CCURL_H */

/*************************************************************************
 * 改动历史纪录：
 * Revision 1.0, 2025-06-11, lium
 * describe: 初始创建.
 *************************************************************************/