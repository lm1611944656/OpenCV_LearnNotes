#include <iostream>
#include <thread>
#include <vector>

#include <curl/curl.h>
#include <opencv2/opencv.hpp>

#include "ccurl.h"
#include "timer.h"

size_t writeData(void *buffer, size_t size, size_t nmemb, void *userp)
{
    std::vector<char> *vec = reinterpret_cast<std::vector<char> *>(userp);
    vec->insert(vec->end(), static_cast<char *>(buffer), static_cast<char *>(buffer) + size * nmemb);
    return size * nmemb;
}

/**
 * @brief 抓取摄像机图像
 * @param _cameraIP 摄像机IP地址
 * @param _username 认证用户名
 * @param _password 认证密码
 * @param _channel 通道号，默认为1
 * @return 图像数据的字节vector
 */
std::vector<char> captureImage(const std::string &_cameraIP, const std::string &_username, const std::string &_password, unsigned int _channel = 1)
{
    // http://xxxxx.xxx.12.228/ISAPI/Streaming/channels/1/picture

    /**定义CURL指针，用于操作libcurl库 */
    CURL *curl;

    /**定义CURL返回码，用于存储请求结果 */
    CURLcode res;
    std::string url = "http://" + _cameraIP + "/ISAPI/Streaming/channels/" + std::to_string(_channel) + "/picture"; // 构建请求URL

    /**存储从摄像头获取的图像数据 */
    std::vector<char> imageData;

    /**初始化libcurl库 */
    curl_global_init(CURL_GLOBAL_DEFAULT);

    /**初始化一个CURL会话 */
    curl = curl_easy_init();

    if (curl)
    { // 如果初始化成功
        // 设置请求的URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // 设置HTTP认证类型为DIGEST
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_DIGEST);

        // 设置用户名和密码
        curl_easy_setopt(curl, CURLOPT_USERPWD, (_username + ":" + _password).c_str());

        // 设置回调函数，用于处理接收到的数据
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);

        // 设置回调函数的用户数据（此处是存储图像数据的vector）
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &imageData);

        // std::cout << "Capturing image from: " << url << std::endl;

        // 执行请求
        res = curl_easy_perform(curl);

        // std::cout << "Image capture result: " << res << std::endl;
        if (res != CURLE_OK) // 如果请求失败
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        else
            std::cout << "Image captured successfully." << std::endl;

        curl_easy_cleanup(curl); // 清理CURL句柄
    }

    std::cout << "Image size: " << imageData.size() << " bytes" << std::endl;
    if (imageData.empty())
    { // 如果没有获取到图像数据
        std::cerr << "Failed to capture image data." << std::endl;
    }
    else
    {
        std::cout << "Image data captured successfully." << std::endl;
    }

    curl_global_cleanup(); // 清理全局libcurl环境
    return imageData;      // 返回获取到的图像数据
}

/**
 * 将图像字节数据转换为cv::Mat对象
 * @param imageData 包含图像数据的字节vector
 * @return 解码后的cv::Mat对象
 */
cv::Mat convertToMat(const std::vector<char> &imageData)
{
    // 使用OpenCV的imdecode函数来解码图像数据
    // 第二个参数-1表示根据图像内容自动推断图像类型
    cv::Mat image = cv::imdecode(cv::Mat(imageData), cv::IMREAD_UNCHANGED);

    if (image.empty())
    { // 如果解码失败
        std::cerr << "Failed to decode image data." << std::endl;
    }
    else
    {
        std::cout << "Image successfully decoded." << std::endl;
    }

    return image;
}

// 更新 generateUniqueFilename 函数以支持毫秒级精度
std::string generateUniqueFilename() {
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto value = now_ms.time_since_epoch();
    long duration = value.count() % 1000; // 获取毫秒数

    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", std::localtime(&now_time));

    std::ostringstream oss;
    oss << "./data/captured_image_" << buffer << "_" << std::setfill('0') << std::setw(3) << duration << ".jpg";
    
    return oss.str();
}

void myCallback2(void)
{
    // 调用captureImage函数获取图像数据
    std::vector<char> imageData = captureImage("10.10.12.228", "admin", "bpg123456");

    // 将图像数据转换为cv::Mat对象
    cv::Mat image = convertToMat(imageData);

    if (!image.empty())
    {
        std::string savePath = generateUniqueFilename();

        // 使用 imwrite 保存图像
        bool success = cv::imwrite(savePath, image);

        if (success)
        {
            std::cout << "Image saved successfully to: " << savePath << std::endl;
        }
        else
        {
            std::cerr << "Failed to save image!" << std::endl;
        }
    }
    else
    {
        std::cerr << "Cannot save empty image." << std::endl;
    }
}

int main(int argc, char **argv)
{
    // auto urlObj = std::make_shared<CCurl>();
    // urlObj->test();

    LibeventTimer timer;
    timer.setTimer(1, true, myCallback2); // 每 20ms 重复调用 myCallback

    // 在新线程中启动定时器
    std::thread timerThread([&]()
                            {
                                timer.start(); // 启动定时器事件循环（阻塞）
                            });

    std::cout << "Press Enter to stop the timer..." << std::endl;
    std::cin.get(); // 等待用户输入回车键来停止定时器

    return 0;
}

#if 0
int main() {
    LibeventTimer timer;
    timer.setTimer(1, true, TaskSchedule); // 每1毫秒秒调用一次 TaskSchedule()

    // 在新线程中启动定时器
    std::thread timerThread([&]() {
        timer.start();   // 启动定时器事件循环（阻塞）
    });

    while (1) {
        TaskHandle(); // 主线程持续检查任务是否可执行
    }
}
#endif