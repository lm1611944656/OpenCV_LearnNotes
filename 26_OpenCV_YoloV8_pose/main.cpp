/*************************************************************************
 *
 *   Copyright (C), 2017-2027, BPG. Co., Ltd.
 *
 *   文件名称: main.cpp
 *   软件模块: 目标检测
 *   版 本 号: 1.0
 *   生成日期: 2025-07-23
 *   作    者: lium
 *   功    能: 关键点检测测试
 *
 ************************************************************************/

 #include "yolov8pose.h"

int main(int argc, char **argv)
{
    std::string modelPath = "weigths/yolov8l-pose.onnx";
    std::string classNamePath = "weigths/yolov8l-pose.txt";
    std::string srcImgPath = "data/bus.jpg";

    std::shared_ptr<CYoloV8Pose> obj = std::make_shared<CYoloV8Pose>(modelPath, classNamePath);
    obj->modelDetect(srcImgPath);

    cv::waitKey(0);
    cv::destroyAllWindows();
}

/*************************************************************************
 * 改动历史纪录：
 * Revision 1.0, 2025-07-23, lium
 * describe: 初始创建.
 *************************************************************************/