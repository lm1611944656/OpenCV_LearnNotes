/*************************************************************************
 *
 *   Copyright (C), 2017-2027, BPG. Co., Ltd.
 *
 *   文件名称: ccurl.cpp
 *   软件模块: 应用模块
 *   版 本 号: 1.0
 *   生成日期: 2025-06-11
 *   作    者: lium
 *   功    能: CURL
 *
 *************************************************************************/
#include "ccurl.h"
#include <iostream>
#include <curl/curl.h> // 包含 libcurl 头文件

CCurl::CCurl()
{
    // 初始化 CURL 库（可选，根据需要调整）
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

CCurl::~CCurl()
{
    // 清理 CURL 库
    curl_global_cleanup();
}

void CCurl::test()
{
    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, "https://www.baidu.com");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        /* always cleanup */
        curl_easy_cleanup(curl);

        if (res == CURLE_OK)
        {
            std::cout << "Response data: " << readBuffer << std::endl;
        }
        else
        {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
    }
}

// 回调函数，用于处理从服务器接收到的数据
size_t CCurl::WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp)
{
    size_t totalSize = size * nmemb;
    userp->append((char *)contents, totalSize);
    return totalSize;
}

/*************************************************************************
 * 改动历史纪录：
 * Revision 1.0, 2025-06-11, lium
 * describe: 初始创建.
 *************************************************************************/