#include <string>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>       // 用于 sleep_for
#include <iostream>     // 用于 std::cerr / std::cout
#include <opencv2/opencv.hpp>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

/**
 * @brief 表示一个网络摄像头（IPC）
 */
class IPCamera {
public:
    /**
     * @param name 摄像头名称（用于标识和推流路径）
     * @param ip 摄像头 IP 地址
     * @param user 登录用户名
     * @param pwd 登录密码
     */
    IPCamera(const std::string& name, const std::string& ip, const std::string& user, const std::string& pwd)
        : cameraName(name), ipAddress(ip), username(user), password(pwd) {}

    /**
     * @return 生成标准 RTSP 拉流 URL（海康/大华常用格式）
     */
    std::string getRtspUrl() const {
        return "rtsp://" + username + ":" + password + "@" + ipAddress + ":554/Streaming/Channels/101";
    }

    /**
     * @return 摄像头名称（如 "GY-Camera1"）
     */
    std::string getCameraName() {
        return this->cameraName;
    }

private:
    std::string cameraName;   ///< 摄像头逻辑名称
    std::string ipAddress;    ///< IP 地址
    std::string username;     ///< 用户名
    std::string password;     ///< 密码
};

/**
 * @brief 获取当前时间字符串，格式： "YYYY-MM-DD HH:MM:SS 星期X"
 * @return std::string 格式化后的时间字符串，如 "2026-01-27 17:35:42 周一"
 */
std::string getCurrentTimeString() {
    auto now = std::chrono::system_clock::now();
    std::time_t time_t_now = std::chrono::system_clock::to_time_t(now);
    
    // 使用 localtime_r / localtime_s 保证线程安全（C++11 起推荐）
    std::tm tm_now{};
#ifdef _WIN32
    localtime_s(&tm_now, &time_t_now);
#else
    localtime_r(&time_t_now, &tm_now);
#endif

    // 星期映射（tm_wday: 0=周日, 1=周一, ..., 6=周六）
    const char* weekdays[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    const char* weekday = weekdays[tm_now.tm_wday];

    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S") << " " << weekday;

    return oss.str();
}

/**
 * @brief 将AVFrame（YUV420）转换为OpenCV Mat格式（BGR）
 */
cv::Mat AVFrameToCVMat(AVFrame *yuv420Frame) {
    // 获取AVFrame信息
    int srcW = yuv420Frame->width;
    int srcH = yuv420Frame->height;
    SwsContext *swsCtx = sws_getContext(srcW, srcH, (AVPixelFormat)yuv420Frame->format, srcW, srcH, AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);

    // 创建一个空的Mat对象
    cv::Mat mat(srcH, srcW, CV_8UC3);

    // 格式转换，直接填充Mat的数据data
    uint8_t* dst_data[1] = { mat.data };
    int dst_linesize[1] = { static_cast<int>(mat.step[0]) };

    // 执行图像格式转换
    sws_scale(swsCtx, yuv420Frame->data, yuv420Frame->linesize, 0, srcH, dst_data, dst_linesize);

    // 释放资源
    sws_freeContext(swsCtx);

    return mat;
}

/**
 * @brief 摄像头管理器：负责多路摄像头的拉流、解码、显示与推流
 */
class CameraManager {
public:
    /**
     * @brief 添加一个摄像头到管理列表
     */
    void addCamera(std::unique_ptr<IPCamera> camera) {
        cameras.emplace_back(std::move(camera));
    }

    /**
     * @brief 启动所有摄像头的拉流线程
     */
    void startAll() {
        for (size_t i = 0; i < cameras.size(); ++i) {
            // 为每个摄像头启动一个独立线程
            threads.emplace_back(&CameraManager::decodeAndShow, this, i);
        }
    }

    /**
     * @brief 等待所有拉流线程结束（阻塞）
     */
    void joinAll() {
        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

private:
    /**
     * @brief 单个摄像头的拉流、解码、图像处理与显示逻辑
     * @param index 摄像头在 cameras 中的索引
     */
    void decodeAndShow(size_t index) {
        auto& cam = cameras[index];
        std::string windowName = "Camera " + std::to_string(index + 1); // OpenCV 窗口标题
        std::string url = cam->getRtspUrl();                            // RTSP 拉流地址

        // 初始化 FFmpeg 网络库（必须调用一次）
        avformat_network_init();

        AVFormatContext* input_ctx = nullptr;
        AVDictionary* opts = nullptr;
        // 强制使用 TCP 传输（更稳定，避免 UDP 丢包）
        av_dict_set(&opts, "rtsp_transport", "tcp", 0);
        // 设置读取超时（单位：微秒，5秒）
        av_dict_set(&opts, "stimeout", "5000000", 0);

        // 打开输入流（连接摄像头）
        int ret = avformat_open_input(&input_ctx, url.c_str(), nullptr, &opts);
        av_dict_free(&opts); // 释放字典
        if (ret < 0) {
            std::cerr << "[ERROR] " << windowName << ": 无法打开输入流: " << url << std::endl;
            return;
        }

        // 获取流信息（探测视频/音频流）
        ret = avformat_find_stream_info(input_ctx, nullptr);
        if (ret < 0) {
            std::cerr << "[ERROR] " << windowName << ": 无法获取流信息" << std::endl;
            avformat_close_input(&input_ctx);
            return;
        }

        // 查找第一个视频流
        int video_index = -1;
        for (unsigned int i = 0; i < input_ctx->nb_streams; ++i) {
            if (input_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                video_index = i;
                break;
            }
        }
        if (video_index == -1) {
            std::cerr << "[ERROR] " << windowName << ": 未找到视频流" << std::endl;
            avformat_close_input(&input_ctx);
            return;
        }

        // 获取视频流的编码参数
        AVCodecParameters* codecpar = input_ctx->streams[video_index]->codecpar;
        // 查找对应的解码器
        const AVCodec* dec_codec = avcodec_find_decoder(codecpar->codec_id);
        if (!dec_codec) {
            std::cerr << "[ERROR] " << windowName << ": 未找到解码器" << std::endl;
            avformat_close_input(&input_ctx);
            return;
        }

        // 分配并配置解码上下文
        AVCodecContext* dec_ctx = avcodec_alloc_context3(dec_codec);
        avcodec_parameters_to_context(dec_ctx, codecpar);
        ret = avcodec_open2(dec_ctx, dec_codec, nullptr);
        if (ret < 0) {
            std::cerr << "[ERROR] " << windowName << ": 无法打开解码器" << std::endl;
            avcodec_free_context(&dec_ctx);
            avformat_close_input(&input_ctx);
            return;
        }

        std::cout << "[INFO] " << windowName << ": 开始解码并显示" << std::endl;

        // 分配数据结构
        AVPacket* pkt = av_packet_alloc();  // 压缩数据包
        AVFrame* frame = av_frame_alloc();  // 解码后的原始帧

        // 主循环：持续读取、解码、显示
        while (true) {
            // 从输入流读取一个 packet
            ret = av_read_frame(input_ctx, pkt);
            if (ret == AVERROR(EAGAIN)) {
                // 暂时无数据，稍等重试
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            if (ret == AVERROR_EOF) {
                // 流结束
                break;
            }
            if (ret < 0) {
                // 读取出错（可能网络中断）
                std::cerr << "[WARN] " << windowName << ": 读取帧失败，尝试重连..." << std::endl;
                avformat_close_input(&input_ctx);
                std::this_thread::sleep_for(std::chrono::seconds(2));

                // 尝试重新连接
                if (avformat_open_input(&input_ctx, url.c_str(), nullptr, nullptr) < 0) {
                    std::cerr << "[ERROR] " << windowName << ": 重连失败，退出" << std::endl;
                    break;
                }
                avformat_find_stream_info(input_ctx, nullptr);
                continue;
            }

            // 跳过非视频流的数据包
            if (pkt->stream_index != video_index) {
                av_packet_unref(pkt);
                continue;
            }

            // 将 packet 发送给解码器
            ret = avcodec_send_packet(dec_ctx, pkt);
            av_packet_unref(pkt); // 释放 packet
            if (ret < 0) continue;

            // 循环接收解码后的帧
            while (ret >= 0) {
                ret = avcodec_receive_frame(dec_ctx, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
                if (ret < 0) break;

                // 使用新函数将 AVFrame 转换为 cv::Mat（BGR）
                cv::Mat bgr_mat = AVFrameToCVMat(frame);

                // 如果转换成功，进行图像处理和显示
                if (!bgr_mat.empty()) {
                    // === 在这里添加你的图像处理逻辑 ===
                    std::string timeStr = getCurrentTimeString();
                    cv::putText(bgr_mat, timeStr, cv::Point(20, 90),
                                cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);
                    cv::rectangle(bgr_mat, cv::Rect(100, 100, 200, 150), cv::Scalar(255, 0, 0), 2);

                    // 显示处理后的帧
                    cv::imshow(windowName, bgr_mat);
                    if (cv::waitKey(1) == 27) { // 按 ESC 退出
                        goto cleanup;
                    }

                    // TODO: 在此处调用 pushVideo() 或 StreamPusher 推流
                    // 例如：
                    // static std::unique_ptr<StreamPusher> pusher;
                    // if (!pusher) {
                    //     std::string pushUrl = "rtmp://10.10.6.233/live/" + cam->getCameraName();
                    //     pusher.reset(new StreamPusher(pushUrl, bgr_mat.cols, bgr_mat.rows, 30));
                    // }
                    // pusher->pushFrame(bgr_mat);
                }
            }
        }

    cleanup:
        // 释放资源
        av_frame_free(&frame);
        av_packet_free(&pkt);
        avcodec_free_context(&dec_ctx);
        avformat_close_input(&input_ctx);
        cv::destroyWindow(windowName); // 只关闭当前窗口
    }

    /**
     * @brief 视频推流接口（预留）
     * 
     * 注意：此函数目前为空。实际推流应封装为独立类（如 StreamPusher），
     * 并在 decodeAndShow 的主循环中调用其 pushFrame() 方法。
     */
    void pushVideo() {
        // 此函数仅为占位符，不建议直接在此实现推流逻辑
        // 推荐方式：在 decodeAndShow 中创建 StreamPusher 实例并调用 pushFrame
    }
    
private:
    std::vector<std::unique_ptr<IPCamera>> cameras; ///< 摄像头列表
    std::vector<std::thread> threads;               ///< 拉流线程列表
};

/**
 * @brief 程序入口
 */
int main() {
    // 关闭 FFmpeg 冗余日志（只显示错误）
    av_log_set_level(AV_LOG_ERROR);

    CameraManager manager;

    // 添加你的摄像头（根据实际情况修改 IP 和账号密码）
    manager.addCamera(std::make_unique<IPCamera>("GY-Camera3", "10.10.182.45", "admin", "bydq123456"));
    manager.addCamera(std::make_unique<IPCamera>("GY-Camera1", "10.10.12.228", "admin", "bpg123456"));
    manager.addCamera(std::make_unique<IPCamera>("GY-Camera2", "10.10.12.224", "admin", "bpg12345"));
    manager.addCamera(std::make_unique<IPCamera>("GY-Camera4", "10.10.6.116", "admin", "bydq2025"));

    // 启动所有摄像头拉流
    manager.startAll();
    // 等待所有线程结束（按 ESC 退出所有窗口后才会返回）
    manager.joinAll();

    // 关闭所有 OpenCV 窗口
    cv::destroyAllWindows();
    return 0;
}