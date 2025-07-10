#include <iostream>
#include "objectdetection.h"

const std::string modelFile = "weight/yolov5s_coco80_20250608.onnx";
const std::string classFile = "weight/yolov5s_coco80_20250608.txt";
const std::string imgPath = "data/Hat3.jpg";
const std::string videoPath = "soucce/sample.mp4";

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