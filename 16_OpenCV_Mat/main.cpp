#include <opencv2/opencv.hpp>
#include <iostream>

const std::string imgPath = "apple.jpg";

/**获取Mat对象的信息 */
static void getMatInfo(cv::Mat &_mat){
    if(_mat.empty()){
        std::cout << "传递参数失败" << std::endl;
        exit(-1);
    }

    /**判断Mat是否是连续对象 */
    if(_mat.isContinuous()){
        std::cout << "Mat是一个连续内存对象!" << std::endl;
    } else {
        std::cout << "Mat不是一个连续内存对象!" << std::endl;
    }

    /**输出Mat的维度 */
    std::cout << "Mat的维度: " << _mat.dims << std::endl;

    /**输出Mat对象的维度 */
    std::cout << "Mat的Shape: " << _mat.size << std::endl;

    /**Mat对象的数据类型 */
    int matType = _mat.type();
    std::cout << "Matrix type: " << matType << std::endl;
}

/**Mat对象进行通道分离 */
static std::vector<cv::Mat> matChannalSplits(cv::Mat &_mat){
    if(_mat.channels() != 3){
        std::cout << "通道不符合要求！" << std::endl;
        return std::vector<cv::Mat>{};
    }

    std::vector<cv::Mat> channalInfo;
    channalInfo.clear();

    cv::split(_mat, channalInfo);
    return channalInfo;
}

/**Mat对象的展示 */
static void showMat(const std::string &winName, cv::Mat &_mat){
    if(_mat.empty()){
        std::cout << "读取图像失败" << std::endl;
        exit(-1);
    }

    cv::namedWindow(winName, cv::WINDOW_AUTOSIZE);
    cv::imshow(winName, _mat);
}

/**图像的像素操作 */
static void imgPixel(cv::Mat &srcImg){
    if(srcImg.empty()){
        std::cout << "参数传递错误! " << std::endl;
        return;
    }

    /**获取图像的信息 */
    int rowCount = srcImg.rows;
    int colCount = srcImg.cols;
    int channalCount = srcImg.channels();

    /**行的起始坐标和结束 */
    int startCoordinate = 100;      /**需要的起始坐标 */
    int endCoordinate = 900;        /**需要的终止坐标 */

    /**需要的目标Mat opencv中的排列为(H, W, C) */
    cv::Mat dstImg(rowCount, endCoordinate - startCoordinate, srcImg.type());

    for(int row = 0; row < rowCount; row++){
        /**创建一个行指针，指向原始的Mat对象 */
        const uchar *pRow = srcImg.ptr<uchar>(row);

        /**创建一个行指针，指向新的Mat对象 */
        uchar *pRowNew = dstImg.ptr<uchar>(row);

        /**指定需要操作的起始地址和终止地址 */
        const uchar *pStart = pRow + (startCoordinate * channalCount);
        const uchar *pEnd = pRow + (endCoordinate * channalCount);

        /**拷贝数据 */
        memcpy(pRowNew, pStart, ((pEnd - pStart) * sizeof(uchar)));
    }
    cv::imshow("Dst Img", dstImg);
}

int main(int argc, char **argv){
    cv::Mat srcImg = cv::imread(imgPath, cv::IMREAD_COLOR);
    if(srcImg.empty()){
        std::cout << "读取图像失败" << std::endl;
        exit(-1);
    }

    //getMatInfo(srcImg);

    /**Mat对象的通道分离 */ 
    std::vector<cv::Mat> channalInfo = matChannalSplits(srcImg);
    std::cout << channalInfo.size() << std::endl;

    /**获取B通道信息 */
    cv::Mat blue = channalInfo[0];
    //getMatInfo(blue);
    // showMat("Blue Win", blue);

    /**获取G通道信息 */
    cv::Mat green = channalInfo[1];
    //getMatInfo(green);
    // showMat("Green Win", green);

    /**获取R通道信息 */
    cv::Mat red = channalInfo[2];
    getMatInfo(red);
    showMat("Red Win", red);
    imgPixel(red);

    cv::waitKey(0);
    cv::destroyAllWindows();
    return 0;
}
