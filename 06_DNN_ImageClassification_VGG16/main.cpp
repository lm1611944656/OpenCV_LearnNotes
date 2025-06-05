#include "imageClassification.h"

/**批处理单张图像 */
#if 1
int main(int argc, char **argv) {
    /**文件路径 */
    std::string imagePath = "data/Foreign_Body_06.jpg";
    std::string modelPath = "weight/vgg16PowerRoomEnvNet.onnx";
    std::string classFilePath = "weight/vgg16PowerRoomEnv_classs.txt";

    /**分类对象创建 */
    ImageClassification imgClass(modelPath, classFilePath);
    
    /**读取图像 */
    cv::Mat srcImg = imgClass.readImg(imagePath);

    /**数据预处理 */
    cv::Mat tensor4D = imgClass.preprocess(srcImg);

    /**模型推理 */
    cv::Mat outTensor = imgClass.modelInference(tensor4D, false);

    /**数据后处理 */
    imgClass.postprocess(outTensor);

    /**绘制预测结果 */
    return 0;
}
#endif

#if 0
#include "imageClassification.h"
#include <opencv2/opencv.hpp>


int main(int argc, char **argv) {
    /**文件路径 */
    std::string imagePath = "data/Foreign_Body_199.jpg";
    std::string imagePath1 = "data/Foreign_Body_06.jpg";
    std::string imagePath2 = "data/Foreign_Body_123.jpg";
    std::string imagePath3 = "data/Maintenance_9165.jpg";
    std::string modelPath = "weight/vgg16PowerRoomEnvNet.onnx";
    std::string classFilePath = "weight/vgg16PowerRoomEnv_classs.txt";

    std::vector<cv::Mat> images;
    images.push_back(cv::imread(imagePath, cv::IMREAD_COLOR));
    images.push_back(cv::imread(imagePath1, cv::IMREAD_COLOR));
    images.push_back(cv::imread(imagePath2, cv::IMREAD_COLOR));
    images.push_back(cv::imread(imagePath3, cv::IMREAD_COLOR));


    /**分类对象创建 */
    ImageClassification imgClass(modelPath, classFilePath);

    /**数据预处理 */
    cv::Mat tensor4DTest = imgClass.preprocess(images);

    /**绘制预测结果 */
    return 0;
}
#endif