#include "distortioncorrection.h"


const std::string imgPath = "data/srcImg.jpeg";

int main(int argv, char **argc){
    CDistortionCorrection *p_obj = new CDistortionCorrection(imgPath);

    cv::namedWindow("cd", cv::WINDOW_AUTOSIZE);
    cv::imshow("cd", p_obj->getBinaryImg());

    cv::waitKey(0);
    cv::destroyAllWindows();
    delete p_obj;
    return 0;
}