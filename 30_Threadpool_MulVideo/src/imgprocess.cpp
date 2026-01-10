#include "imgprocess.h"


CImgProcess::CImgProcess()
{

}

CImgProcess::~CImgProcess()
{

}

void CImgProcess::run(cv::Mat &srcImg, cv::Mat &resultImg)
{
    // 确保源图像不是空的
    if (srcImg.empty()) {
        std::cerr << "src NULL" << std::endl;
        return;
    }

    static int id = 0;
    id++; // 帧编号递增

    // 复制源图像到结果图像
    srcImg.copyTo(resultImg);

    // 要绘制的文本（包含帧ID）
    std::string text = "You are Qwen, created by Alibaba Cloud. You are a helpful assistant. Frame: " + std::to_string(id);

    // 字体设置
    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    double fontScale = 1.0;           // 放大字体
    int thickness = 2;                // 线条粗细
    cv::Scalar color = cv::Scalar(0, 255, 0); // 绿色
    int baseline = 0;

    // 获取文本尺寸
    cv::Size textSize = cv::getTextSize(text, fontFace, fontScale, thickness, &baseline);

    // 判断是否太宽，自动换行处理
    int maxLineWidth = resultImg.cols - 40; // 左右留白 20px
    if (textSize.width > maxLineWidth) {
        // 启用换行
        drawTextMultiLine(resultImg, text, cv::Point(20, 60), fontFace, fontScale, color, thickness);
    } else {
        // 单行居中显示
        cv::Point textOrg((resultImg.cols - textSize.width) / 2,
                          (resultImg.rows + textSize.height) / 2);
        cv::putText(resultImg, text, textOrg, fontFace, fontScale, color, thickness);
    }
}

// 工具函数：自动换行绘制文本
void CImgProcess::drawTextMultiLine(cv::Mat& img, const std::string& text, cv::Point org,
                       int fontFace, double fontScale, const cv::Scalar& color, int thickness)
{
    std::istringstream iss(text);
    std::string line, word;
    std::vector<std::string> lines;
    std::string current_line = "";

    // 按空格拆分单词
    while (iss >> word) {
        std::string test_line = current_line + (current_line.empty() ? "" : " ") + word;
        cv::Size sz = cv::getTextSize(test_line, fontFace, fontScale, thickness, nullptr);
        if (sz.width > img.cols - 40) {
            if (!current_line.empty()) {
                lines.push_back(current_line);
                current_line = word;
            } else {
                lines.push_back(test_line);
                current_line = "";
            }
        } else {
            current_line = test_line;
        }
    }
    if (!current_line.empty()) {
        lines.push_back(current_line);
    }

    int lineHeight = cv::getTextSize("Ag", fontFace, fontScale, thickness, nullptr).height + 10;

    for (size_t i = 0; i < lines.size(); ++i) {
        cv::Point lineOrg(org.x, org.y + i * lineHeight);
        cv::putText(img, lines[i], lineOrg, fontFace, fontScale, color, thickness);
    }
}