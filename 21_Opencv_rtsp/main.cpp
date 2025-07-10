#include <opencv2/opencv.hpp>
#include <iostream>

int main(int argc, char **argv) {
    // 设置 RTSP 流地址
    std::string rtsp_url1 = "rtsp://admin:bpg123456@10.10.12.228:554/h264/2/main/av_stream";
    std::string rtsp_url2 = "rtsp://admin:bydq123456@10.10.182.45:554/h264/2/main/av_stream";

    cv::VideoCapture cap(rtsp_url2, cv::CAP_FFMPEG);

    if (!cap.isOpened()) {
        std::cerr << "无法打开 RTSP 流!" << std::endl;
        return -1;
    }

    // 获取视频帧的宽度和高度
    double frame_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    double frame_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    std::cout << "视频帧宽度: " << frame_width << ", 视频帧高度: " << frame_height << std::endl;

    cv::Mat frame;
    int frame_count = 0;
    double start_time = (double)cv::getTickCount();  // 开始计时

    while (true) {
        cap >> frame;

        if (frame.empty()) {
            std::cerr << "无法读取帧 (断开连接？)" << std::endl;
            break;
        }

        frame_count++;

        // 计算 FPS
        double current_time = (double)cv::getTickCount();
        double elapsed_time = (current_time - start_time) / cv::getTickFrequency();  // 秒

        if (elapsed_time >= 1.0) {  // 每1秒更新一次FPS
            double fps = frame_count / elapsed_time;
            std::cout << "当前 FPS: " << fps << std::endl;
            frame_count = 0;
            start_time = current_time;
        }

        // 在图像上绘制 FPS 文本
        double fps_to_display = frame_count / elapsed_time;
        std::string fps_text = "FPS: " + cv::format("%.2f", fps_to_display);

        cv::putText(frame, fps_text,
                    cv::Point(20, 40),                  // 文字位置
                    cv::FONT_HERSHEY_SIMPLEX,           // 字体
                    1.0,                                // 字号
                    cv::Scalar(0, 255, 0),              // 颜色（绿色）
                    2);                                 // 粗细

        // 显示图像
        cv::imshow("RTSP Stream", frame);

        // 按 ESC 键退出
        if (cv::waitKey(1) == 27) {
            break;
        }
    }

    cap.release();
    cv::destroyAllWindows();

    return 0;
}