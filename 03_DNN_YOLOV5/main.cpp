#include <iostream>
#include "objectdetection.h"

const std::string modelPath = "/home/lmz/workspace/opencv_workspace/OpenCV_LearnNotes/03_DNN_YOLOV5/yolov5s.onnx";
const std::string classPath = "/home/lmz/workspace/opencv_workspace/OpenCV_LearnNotes/03_DNN_YOLOV5/classes.txt";
const std::string imgPath = "/home/lmz/workspace/opencv_workspace/OpenCV_LearnNotes/03_DNN_YOLOV5/image.png";


int main(int argc, char **argv){
    std::cout << "Model Path: " << modelPath << std::endl;
    std::cout << "Class Path: " << classPath << std::endl;
    std::cout << "Image Path: " << imgPath << std::endl;
    //ObjectDetection *obj = new ObjectDetection(modelPath, classPath, imgPath);
    // std::cout << "对象构造完成！" << std::endl;

    ObjectDetection obj(modelPath, classPath, imgPath);
    std::cout << "对象构造完成！" << std::endl;

    obj.modelInfer();

    //obj.camel_elephant_infer(modelPath, classPath);
    waitKey(0);
	destroyAllWindows();
    return 0;
}