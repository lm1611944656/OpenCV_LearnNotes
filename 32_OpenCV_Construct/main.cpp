#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <map>
#include <vector>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <ctime>
#include <opencv2/opencv.hpp>

#include "thread_safe_queue.h"
#include "base_camera.h"
#include "ip_camera.h"
#include "camera_manager.h"


#if 0
/** 测试示例，用自定义的MockCamera摄像头，模拟相机。验证推理框架 */
int main() {
	try {
		// 1. 创建相机管理器
		CameraManager manager;

		// 2. 添加相机（模拟相机，替换为USBCamera(0)可测试真实USB相机）
		manager.addCamera(std::make_unique<MockCamera>("mock_0", cv::Size(640, 480)));
		manager.addCamera(std::make_unique<MockCamera>("mock_1", cv::Size(640, 480)));
		manager.addCamera(std::make_unique<MockCamera>("mock_2", cv::Size(640, 480)));
		// manager.addCamera(std::make_unique<USBCamera>(0, cv::Size(640, 480), 30)); // 真实USB相机

		// 3. 初始化所有相机
		if (!manager.initAllCameras()) {
			std::cerr << "[ERROR] Camera init failed!" << std::endl;
			return -1;
		}

		// 4. 启动所有相机采集
		if (!manager.startAllCapture()) {
			std::cerr << "[ERROR] Capture start failed!" << std::endl;
			return -1;
		}

		// 5. 循环获取并显示图像
		std::cout << "[INFO] Start capturing... Press 'q' to quit!" << std::endl;
		while (true) {
			// 获取所有相机的最新帧
			auto all_frames = manager.getAllFrames();

			// 显示每个相机的图像
			for (auto& [cam_id, frame_data] : all_frames) {
				if (frame_data.frame.empty()) {
					continue;
				}

				// 绘制时间戳（格式化，避免换行）
				std::time_t ts = static_cast<std::time_t>(frame_data.timestamp);
				char time_buf[64];
				std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", std::localtime(&ts));
				std::string time_str(time_buf);
				
				cv::putText(frame_data.frame, "Time: " + time_str,
					cv::Point(10, 60), cv::FONT_HERSHEY_SIMPLEX,
					1.0, cv::Scalar(0, 0, 255), 2);

				// 显示窗口
				cv::imshow("Camera " + cam_id, frame_data.frame);
			}

			// 按'q'退出（注意waitKey的延迟，避免CPU占用过高）
			int key = cv::waitKey(1);
			if (key == 'q' || key == 27) { // 27是ESC键
				break;
			}
		}

		// 6. 停止采集并释放资源
		manager.stopAllCapture();
		manager.closeAllCameras();
		cv::destroyAllWindows();
		std::cout << "[INFO] Test finished!" << std::endl;
	}
	catch (const std::exception & e) {
		std::cerr << "[ERROR] Test exception: " << e.what() << std::endl;
		return -1;
	}

	return 0;
}

#endif

#if 0
/** 测试示例，将真实摄像头的视频流获取并且保存到本地 */
int main() {
    try {
        // 1. 创建相机管理器
        CameraManager manager;

        // 2. 添加你的三个 RTSP 摄像头
        manager.addCamera(std::make_unique<IPCamera>("GY-Camera1", "10.10.12.228", "admin", "bpg123456"));
        manager.addCamera(std::make_unique<IPCamera>("GY-Camera2", "10.10.12.224", "admin", "bpg12345"));
        manager.addCamera(std::make_unique<IPCamera>("GY-Camera3", "10.10.182.45", "admin", "bydq123456"));

        // 3. 初始化所有相机
        if (!manager.initAllCameras()) {
            std::cerr << "[ERROR] Camera initialization failed!" << std::endl;
            return -1;
        }

        // 4. 启动所有采集线程
        if (!manager.startAllCapture()) {
            std::cerr << "[ERROR] Failed to start capture!" << std::endl;
            return -1;
        }

        // 5. 【关键修复】创建可靠的 VideoWriter（使用 .avi + MJPG）
        std::map<std::string, cv::VideoWriter> writers;
        const int FPS = 25;
        const auto fourcc = cv::VideoWriter::fourcc('M', 'J', 'P', 'G'); // Motion JPEG
        const cv::Size SAVE_SIZE(1920, 1080); // 统一分辨率（根据你的摄像头调整）

        std::vector<std::string> cam_ids = {"GY-Camera1", "GY-Camera2", "GY-Camera3"};
        for (const auto& id : cam_ids) {
            std::string filename = id + ".avi"; // 必须是 .avi
            cv::VideoWriter writer(filename, fourcc, FPS, SAVE_SIZE);

            if (!writer.isOpened()) {
                std::cerr << "[ERROR] Cannot create video file: " << filename << std::endl;
                continue;
            }

            writers[id] = std::move(writer);
            std::cout << "[INFO] Saving video to: " << filename << std::endl;
        }

        // 6. 主循环：获取帧 → 调整尺寸 → 保存
        std::cout << "\n[INFO] Recording started. Press 'q' to stop and save.\n";
        while (true) {
            auto all_frames = manager.getAllFrames(); // 非阻塞获取

            for (auto& [cam_id, frame_data] : all_frames) {
                if (frame_data.frame.empty()) continue;

                // 强制调整到统一尺寸（必须！）
                cv::Mat resized_frame;
                cv::resize(frame_data.frame, resized_frame, SAVE_SIZE);

                // 保存到对应摄像头的文件
                if (writers.find(cam_id) != writers.end()) {
                    writers[cam_id].write(resized_frame);
                }

                // 可选：显示实时画面
                cv::imshow("Live: " + cam_id, resized_frame);
            }

            // 按 'q' 退出
            if (cv::waitKey(1) == 'q') {
                break;
            }
        }

        // 7. 清理资源：释放 VideoWriter（触发写入文件尾部）
        std::cout << "\n[INFO] Finalizing video files..." << std::endl;
        for (auto& [cam_id, writer] : writers) {
            if (writer.isOpened()) {
                writer.release();
                std::cout << "[SUCCESS] Saved: " << cam_id << ".avi" << std::endl;
            }
        }

        // 关闭摄像头和窗口
        manager.stopAllCapture();
        manager.closeAllCameras();
        cv::destroyAllWindows();

        std::cout << "\n[ALL DONE] Videos are ready to play with any media player!\n";
    }
    catch (const std::exception& e) {
        std::cerr << "[FATAL ERROR] " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
#endif

#if 0
/** 测试示例，将真实摄像头的视频流进行视频帧处理，处理之后再显示 */
int main() {
    try {
        // 1. 创建相机管理器
        CameraManager manager;

        // 2. 添加你的三个 RTSP 摄像头
        manager.addCamera(std::make_unique<IPCamera>("GY-Camera1", "10.10.12.228", "admin", "bpg123456"));
        manager.addCamera(std::make_unique<IPCamera>("GY-Camera2", "10.10.12.224", "admin", "bpg12345"));
        manager.addCamera(std::make_unique<IPCamera>("GY-Camera3", "10.10.182.45", "admin", "bydq123456"));

        // 3. 初始化所有相机
        if (!manager.initAllCameras()) {
            std::cerr << "[ERROR] Camera initialization failed!" << std::endl;
            return -1;
        }

        // 4. 启动所有采集线程
        if (!manager.startAllCapture()) {
            std::cerr << "[ERROR] Failed to start capture!" << std::endl;
            return -1;
        }

        std::cout << "\n[INFO] Displaying live streams with ID and timestamp. Press 'q' to quit.\n";

        // 5. 主循环：获取帧 → 叠加文字 → 显示
        while (true) {
            auto all_frames = manager.getAllFrames();

            for (auto& [cam_id, frame_data] : all_frames) {
                if (frame_data.frame.empty()) {
                    // 可选：显示黑屏提示
                    cv::Mat black(480, 640, CV_8UC3, cv::Scalar(0, 0, 0));
                    cv::putText(black, cam_id + ": No Signal", cv::Point(10, 240),
                                cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 255), 2);
                    cv::imshow("Live: " + cam_id, black);
                    continue;
                }

                cv::Mat frame = frame_data.frame.clone();

                // 获取当前时间字符串
                auto now = std::chrono::system_clock::now();
                auto time_t = std::chrono::system_clock::to_time_t(now);
                std::string timestamp = std::ctime(&time_t);
                if (!timestamp.empty() && timestamp.back() == '\n') {
                    timestamp.pop_back(); // 移除末尾换行符
                }

                // 在帧上绘制摄像头 ID 和时间戳
                cv::putText(frame, cam_id, cv::Point(10, 30),
                            cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
                cv::putText(frame, timestamp, cv::Point(10, 60),
                            cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 2);

                // 显示
                cv::imshow("Live: " + cam_id, frame);
            }

            // 按 'q' 或 ESC 退出
            int key = cv::waitKey(1) & 0xFF;
            if (key == 'q' || key == 27) {
                break;
            }
        }

        // 6. 清理
        manager.stopAllCapture();
        manager.closeAllCameras();
        cv::destroyAllWindows();

        std::cout << "\n[INFO] Application exited.\n";
    }
    catch (const std::exception& e) {
        std::cerr << "[FATAL] " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
#endif

#if 0
/** 测试示例，系统日志框架验证*/
#include "logger.h"
#include <stdio.h>

#define LOG_MODULE_NAME "NETWORK"

int main(void) {
    log_set_level(LOG_LEVEL_ERROR);
    log_set_output_file("app.log");

    LOG_DEBUG("Starting network thread...");
    LOG_INFO("Connected to 192.168.1.100");
    LOG_WARN("Latency: %d ms", 150);
    LOG_ERROR("Timeout on packet #%d", 42);
    // LOG_FATAL("Critical hardware failure!");

    printf("\n Check 'app.log' for full output.\n");
    return 0;
}
#endif

#if 1
/** 测试示例，框架中加入了帧率统计、队列积压告警、丢帧计数*/
int main() {
    try {
        // 1. 创建相机管理器
        CameraManager manager;

        // 2. 添加你的三个 RTSP 摄像头
        manager.addCamera(std::make_unique<IPCamera>("GY-Camera1", "10.10.12.228", "admin", "bpg123456"));
        manager.addCamera(std::make_unique<IPCamera>("GY-Camera2", "10.10.12.224", "admin", "bpg12345"));
        manager.addCamera(std::make_unique<IPCamera>("GY-Camera3", "10.10.182.45", "admin", "bydq123456"));

        // 3. 初始化所有相机
        if (!manager.initAllCameras()) {
            std::cerr << "[ERROR] Camera initialization failed!" << std::endl;
            return -1;
        }

        // 4. 启动所有采集线程
        if (!manager.startAllCapture()) {
            std::cerr << "[ERROR] Failed to start capture!" << std::endl;
            return -1;
        }

        std::cout << "\n[INFO] Displaying live streams with ID and timestamp. Press 'q' to quit.\n";

        // 5. 主循环：获取帧 → 叠加文字 → 显示
        auto last_metric_time = std::chrono::steady_clock::now();
        while (true) {
            auto all_frames = manager.getAllFrames();

            for (auto& [cam_id, frame_data] : all_frames) {
                if (frame_data.frame.empty()) {
                    // 可选：显示黑屏提示
                    cv::Mat black(480, 640, CV_8UC3, cv::Scalar(0, 0, 0));
                    cv::putText(black, cam_id + ": No Signal", cv::Point(10, 240),
                                cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 255), 2);
                    //cv::imshow("Live: " + cam_id, black);
                    continue;
                }

                cv::Mat frame = frame_data.frame.clone();

                // 获取当前时间字符串
                // auto now = std::chrono::system_clock::now();
                // auto time_t = std::chrono::system_clock::to_time_t(now);
                // std::string timestamp = std::ctime(&time_t);
                // if (!timestamp.empty() && timestamp.back() == '\n') {
                //     timestamp.pop_back(); // 移除末尾换行符
                // }

                // 在帧上绘制摄像头 ID 和时间戳
                // cv::putText(frame, cam_id, cv::Point(10, 30),
                //             cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
                // cv::putText(frame, timestamp, cv::Point(10, 60),
                //             cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 2);

                // 显示
                cv::imshow("Live: " + cam_id, frame);
            }

            // 每2秒打印性能
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration<double>(now - last_metric_time).count() >= 2.0) {
                manager.printAllMetrics(); 
                last_metric_time = now;
            }

            // 按 'q' 或 ESC 退出
            int key = cv::waitKey(1) & 0xFF;
            if (key == 'q' || key == 27) {
                break;
            }
        }

        // 6. 清理
        manager.stopAllCapture();
        manager.closeAllCameras();
        cv::destroyAllWindows();

        std::cout << "\n[INFO] Application exited.\n";
    }
    catch (const std::exception& e) {
        std::cerr << "[FATAL] " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
#endif