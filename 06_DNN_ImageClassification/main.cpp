#include "imageClassification.h"

int main(int argc, char **argv) {
    /**文件路径 */
    std::string imagePath = "data/DoorOpened_131.jpg";
    std::string modelPath = "weight/best.onnx";
    std::string classFilePath = "weight/classs.txt";

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