#include "objectDetect.h"

std::string modelPath = "weigth/yolov5s.onnx";
std::string classPath = "weigth/classs.txt"; 
std::string imgPath = "data/bus.jpg";
std::string videoPath = "data/car_count_video.mp4";


int main(int argc, char **argv){
    CObjeectDetect *m_carDetect = new CObjeectDetect(modelPath, classPath);

    /**图像检测 */
    m_carDetect->imageInference(imgPath);

    /**视频检测 */
    //m_carDetect->videoInference(videoPath);

    /**车辆的数量计数 */
    //m_carDetect->vehicleCounting(videoPath);

    cv::waitKey(0);
    cv::destroyAllWindows();
    return 0;
}