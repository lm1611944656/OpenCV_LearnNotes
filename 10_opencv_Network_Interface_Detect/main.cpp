#include <iostream>
#include "networkInterfaceDetect.h"

const std::string imgPath = "data/srcImg.png";

int main(void){
    CNetworkInterfaceDetect *m_objDetect = new CNetworkInterfaceDetect(imgPath);
    
    cv::waitKey(0);
    cv::destroyAllWindows();
    return 0;
}