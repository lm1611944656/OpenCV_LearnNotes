#include <opencv2/opencv.hpp>
#include <iostream>

// 全局变量，用于存储图像
cv::Mat img;

// 鼠标回调函数
void on_EVENT_LBUTTONDOWN(int event, int x, int y, int flags, void* userdata) {
    if (event == cv::EVENT_LBUTTONDOWN) {
        // 打印点击的坐标
        std::cout << "Clicked at: (" << x << ", " << y << ")" << std::endl;

        // 在点击位置绘制一个红色圆点
        cv::circle(img, cv::Point(x, y), 2, cv::Scalar(0, 255, 0), -1);

        // 将坐标转换为字符串并绘制到图像上
        std::string xy = std::to_string(x) + "," + std::to_string(y);
        cv::putText(img, xy, cv::Point(x, y), cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar(0, 255, 0), 1);

        // 更新窗口以显示变化
        cv::imshow("image", img);
    }
}

int main() {
    // 读取图片
    img = cv::imread("DoorOpened_88.jpg");
    if (img.empty()) {
        std::cerr << "Error: Could not load image!" << std::endl;
        return -1;
    }

    // 创建窗口并绑定鼠标回调函数
    cv::namedWindow("image");
    cv::setMouseCallback("image", on_EVENT_LBUTTONDOWN);

    // 显示图片
    cv::imshow("image", img);

    // 等待用户按键操作
    while (true) {
        char key = cv::waitKey(5) & 0xFF;
        if (key == 'q') {  // 按下 'q' 键退出
            break;
        }
    }

    // 关闭所有窗口
    cv::destroyAllWindows();

    return 0;
}