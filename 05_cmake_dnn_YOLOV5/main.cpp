#include <iostream>
#include "objectdetection.h"

const std::string modelFile = "/home/lmz/workspace/opencv_workspace/OpenCV_LearnNotes/05_cmake_dnn_YOLOV5/soucce/yolov5s.onnx";
const std::string classFile = "/home/lmz/workspace/opencv_workspace/OpenCV_LearnNotes/05_cmake_dnn_YOLOV5/soucce/classes.txt";
const std::string imgPath = "soucce/carImg.jpg";
const std::string videoPath = "/home/lmz/workspace/opencv_workspace/OpenCV_LearnNotes/05_cmake_dnn_YOLOV5/soucce/sample.mp4";

int main(int argc, char **argv){
    /** 创建检测对象 */
    YoloV5CPP yoloObj(modelFile, classFile);

    /** 图像检测 */
    yoloObj.modelInferenceOfImg(imgPath);

    /** 视频检测 */
    //yoloObj.modelInferenceOfVideo(videoPath);

    cv::waitKey(0);
	cv::destroyAllWindows();
    return 0;
}