#include "imagesegmentation.h"


std::string modelPath = "weigths/yolov5s-seg.onnx";
std::string classLabelPath = "data/classs.txt";
std::string imgPath = "data/bus.jpg";
std::string videoPath = "data/che.avi";

int main(int argc, char **argv){
    ImgSegmentation *m_imgSegmentation = new ImgSegmentation(modelPath, classLabelPath, imgPath);
    /**图像推理检测 */
    //m_imgSegmentation->imageInferenceDetection();

    /**视频推理检测 */
    //m_imgSegmentation->videoInferenceDetection(videoPath);

    std::shared_ptr<ImgSegmentation> m_imgSeg = std::make_shared<ImgSegmentation>(modelPath, classLabelPath, imgPath);
    m_imgSeg->videoInferenceDetection(videoPath);
    

    cv::waitKey(0);
    cv::destroyAllWindows();
    return 0;
}