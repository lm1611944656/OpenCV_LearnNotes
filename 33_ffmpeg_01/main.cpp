#include <string>
#include <memory>
#include <vector>
#include <thread>
#include <memory>
#include <opencv2/opencv.hpp>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

class IPCamera {
public:
    IPCamera(const std::string& name, const std::string& ip, const std::string& user, const std::string& pwd)
        : cameraName(name), ipAddress(ip), username(user), password(pwd) {}

    std::string getRtspUrl() const {
        return "rtsp://" + username + ":" + password + "@" + ipAddress + ":554/Streaming/Channels/101";
    }

    std::string getCameraName()
    {
        return this->cameraName;
    }

private:
    std::string cameraName;
    std::string ipAddress;
    std::string username;
    std::string password;
};



class CameraManager {
public:
    void addCamera(std::unique_ptr<IPCamera> camera) {
        cameras.emplace_back(std::move(camera));
    }

    void startAll() {
        for (size_t i = 0; i < cameras.size(); ++i) {
            threads.emplace_back(&CameraManager::decodeAndShow, this, i);
        }
    }

    void joinAll() {
        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

private:
    /**拉流与展示 */
    void decodeAndShow(size_t index) {
        auto& cam = cameras[index];
        std::string windowName = "Camera " + std::to_string(index + 1);
        std::string url = cam->getRtspUrl();

        avformat_network_init();

        AVFormatContext* input_ctx = nullptr;
        AVDictionary* opts = nullptr;
        av_dict_set(&opts, "rtsp_transport", "tcp", 0);
        av_dict_set(&opts, "stimeout", "5000000", 0); // 5秒超时

        int ret = avformat_open_input(&input_ctx, url.c_str(), nullptr, &opts);
        av_dict_free(&opts);
        if (ret < 0) {
            std::cerr << "[ERROR] " << windowName << ": 无法打开输入流: " << url << std::endl;
            return;
        }

        ret = avformat_find_stream_info(input_ctx, nullptr);
        if (ret < 0) {
            std::cerr << "[ERROR] " << windowName << ": 无法获取流信息" << std::endl;
            avformat_close_input(&input_ctx);
            return;
        }

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

        AVCodecParameters* codecpar = input_ctx->streams[video_index]->codecpar;
        const AVCodec* dec_codec = avcodec_find_decoder(codecpar->codec_id);
        if (!dec_codec) {
            std::cerr << "[ERROR] " << windowName << ": 未找到解码器" << std::endl;
            avformat_close_input(&input_ctx);
            return;
        }

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

        AVPacket* pkt = av_packet_alloc();
        AVFrame* frame = av_frame_alloc();
        SwsContext* sws_ctx = nullptr;

        while (true) {
            ret = av_read_frame(input_ctx, pkt);
            if (ret == AVERROR(EAGAIN)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            if (ret == AVERROR_EOF) break;
            if (ret < 0) {
                std::cerr << "[WARN] " << windowName << ": 读取帧失败，尝试重连..." << std::endl;
                avformat_close_input(&input_ctx);
                std::this_thread::sleep_for(std::chrono::seconds(2));
                if (avformat_open_input(&input_ctx, url.c_str(), nullptr, nullptr) < 0) {
                    std::cerr << "[ERROR] " << windowName << ": 重连失败，退出" << std::endl;
                    break;
                }
                avformat_find_stream_info(input_ctx, nullptr);
                continue;
            }

            if (pkt->stream_index != video_index) {
                av_packet_unref(pkt);
                continue;
            }

            ret = avcodec_send_packet(dec_ctx, pkt);
            av_packet_unref(pkt);
            if (ret < 0) continue;

            while (ret >= 0) {
                ret = avcodec_receive_frame(dec_ctx, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
                if (ret < 0) break;

                cv::Mat bgr_mat(frame->height, frame->width, CV_8UC3);
                int dst_linesize[1] = { static_cast<int>(bgr_mat.step[0]) };

                sws_ctx = sws_getCachedContext(sws_ctx,
                                               frame->width, frame->height, (AVPixelFormat)frame->format,
                                               frame->width, frame->height, AV_PIX_FMT_BGR24,
                                               SWS_BILINEAR, nullptr, nullptr, nullptr);

                sws_scale(sws_ctx,
                          frame->data, frame->linesize,
                          0, frame->height,
                          &bgr_mat.data, dst_linesize);

                if (!bgr_mat.empty()) {
                    // === 在这里添加你的图像处理 ===
                    cv::putText(bgr_mat, cam->getCameraName(), cv::Point(20, 50),
                                cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
                    cv::rectangle(bgr_mat, cv::Rect(100, 100, 200, 150), cv::Scalar(255, 0, 0), 2);

                    
                    // 显示处理后的帧
                    cv::imshow(windowName, bgr_mat);
                    if (cv::waitKey(1) == 27) {
                        goto cleanup;
                    }
                }
            }
        }

    cleanup:
        sws_freeContext(sws_ctx);
        av_frame_free(&frame);
        av_packet_free(&pkt);
        avcodec_free_context(&dec_ctx);
        avformat_close_input(&input_ctx);
        cv::destroyWindow(windowName); // 只关闭自己的窗口
    }

    /**视频推流 */
    void pushVideo(){

    }
    
private:
    std::vector<std::unique_ptr<IPCamera>> cameras;
    std::vector<std::thread> threads;
};

int main() {
    av_log_set_level(AV_LOG_ERROR);

    CameraManager manager;

    // 添加你的摄像头
    manager.addCamera(std::make_unique<IPCamera>("GY-Camera3", "10.10.182.45", "admin", "bydq123456"));
    manager.addCamera(std::make_unique<IPCamera>("GY-Camera1", "10.10.12.228", "admin", "bpg123456"));
    manager.addCamera(std::make_unique<IPCamera>("GY-Camera2", "10.10.12.224", "admin", "bpg12345"));
    manager.addCamera(std::make_unique<IPCamera>("GY-Camera4", "10.10.6.116", "admin", "bydq2025"));
    // 如果有更多摄像头，可以继续添加

    manager.startAll();
    manager.joinAll();

    cv::destroyAllWindows();
    return 0;
}