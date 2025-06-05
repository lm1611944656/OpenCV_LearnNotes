#include "imageClassification.h"
#include <fstream>


ImageClassification::ImageClassification(const std::string &modelFile, const std::string &classFiles)
:m_modelFile(modelFile), m_classFile(classFiles)
{
    deInit();

    /** 加载类别标签 */
    loadClassLabel();
}


ImageClassification::~ImageClassification(){

}

cv::Mat ImageClassification::softmax(const cv::Mat& logits) 
{ 
    cv::Mat result = logits.clone(); // 复制输入矩阵
    double maxVal = *std::max_element(result.begin<float>(), result.end<float>());  // 防止数值溢出
    double expSum = 0.0;

    // 计算指数和
    for (int i = 0; i < result.cols; ++i) {
        float expVal = exp(result.at<float>(0, i) - maxVal);
        result.at<float>(0, i) = expVal;
        expSum += expVal;
    }

    // 归一化
    for (int i = 0; i < result.cols; ++i) {
        result.at<float>(0, i) /= expSum;
    }

    return result; // 返回归一化后的矩阵
}

void ImageClassification::deInit(void){
    classLabel.clear();
}


void ImageClassification::loadClassLabel(void){
    if(m_classFile.empty()){
        throw std::invalid_argument("打开文件路径为空！");
    }

    /**打开文件夹 */
    std::ifstream file(m_classFile);
    if (!file) {
        throw std::runtime_error("无法打开文件: " + m_classFile);
    }
    
    /**获取文件的每一行 */
    std::string className;
    while(std::getline(file, className)){
        if(!className.empty()){
            this->classLabel.push_back(className);
        }
    }

    /**关闭文件夹 */
    file.close();
}

cv::Mat ImageClassification::readImg(const std::string &imgPath){
    if(imgPath.empty()){
        throw std::invalid_argument("打开文件路径为空！");
    }

    cv::Mat srcImg = cv::imread(imgPath, cv::IMREAD_COLOR);
    if(srcImg.empty()){
        throw std::invalid_argument("打开图像失败！");
    }
    return srcImg;
}

cv::Mat ImageClassification::preprocess(cv::Mat &srcImg){
    if(srcImg.empty()){
        std::cout << __FILE__ << " " << __LINE__ << std::endl;
        throw std::invalid_argument("输入的Mat对象为空! ");
    }

    std::cout << "输入图像的维度: " <<  srcImg.dims << std::endl;
    std::cout << "输入图像的形状: " <<  srcImg.size << std::endl;

    cv::Mat tensor4D;   // 需要的Tensor4D [1 x 3 x 224 x 224]

    /**原始图像需要乘于的缩放因子 */
    double scalefactor = 1.0 / 255.0;  
    
    /**原始图像需要乘于缩放因子之后，需要减去的均值 */
    cv::Scalar mean = cv::Scalar(0, 0, 0); 

    /**由于模型需要的是RGB，但是opencv是BGR */
    bool swapRB = true; // 如果模型需要 BGR -> RGB 转换
    cv::dnn::blobFromImage(srcImg, tensor4D, scalefactor, cv::Size(width, height), mean, swapRB, false);
    
    /**输入tensor的形状 */
    // std::cout << "输入tensor的维度: " <<  tensor4D.dims << std::endl;
    std::cout << "输入tensor的形状: " <<  tensor4D.size << std::endl;// 1 x 3 x 224 x 224


    // 打印 tensor4D 的形状
    // std::cout << "Blob shape: [" 
    // << tensor4D.size[0] << ", "     // 获取第0维度的数据
    // << tensor4D.size[1] << ", "     // 获取第1维度的数据
    // << tensor4D.size[2] << ", "     // 获取第2维度的数据
    // << tensor4D.size[3] << "]"      // 获取第3维度的数据
    // << std::endl;

    return tensor4D;
}

cv::Mat ImageClassification::preprocess(std::vector<cv::Mat> inputSrcImg){
    if(inputSrcImg.size() == 0){
        std::cout << __FILE__ << " " << __LINE__ << std::endl;
        throw std::invalid_argument("输入的Mat对象为空! ");
    }

    // 参数设置
    double scalefactor = 1.0 / 255.0;  
    cv::Scalar mean = cv::Scalar(0, 0, 0); 
    bool swapRB = true;

    // 使用 blobFromImages 处理多个图像
    cv::Mat tensor4D;
    std::vector<cv::Mat> processedImages;
    for (const auto& img : inputSrcImg) {
        if (img.empty()) {
            std::cout << __FILE__ << " " << __LINE__ << std::endl;
            throw std::invalid_argument("某张输入的Mat对象为空!");
        }
        processedImages.push_back(img);
    }

    // 创建 batch blob
    cv::dnn::blobFromImages(processedImages, tensor4D, scalefactor, cv::Size(width, height), mean, swapRB, false);
    std::cout << "输入tensor的形状: " <<  tensor4D.size << std::endl;// 4 x 3 x 224 x 224
    
    // 输出 tensor4D 的形状
    std::cout << "输入tensor的形状: " << tensor4D.size[0] << ", "
                << tensor4D.size[1] << ", "
                << tensor4D.size[2] << ", "
                << tensor4D.size[3] << std::endl;

    return tensor4D;
}

cv::Mat ImageClassification::modelInference(cv::Mat &tensor4D, bool isUsedCUDA){
    if(tensor4D.empty()){
        throw std::invalid_argument("输入的Mat对象为空! ");
    }

    /**读取网络模型 */
    cv::dnn::Net model = cv::dnn::readNet(this->m_modelFile);
    if (model.empty()) {
        throw std::runtime_error("无法加载模型文件: " + m_modelFile);
    }

    /**读取模型的相关信息 */
    // getNetModelInfo(model);

    /** 判断是是GPU和CPU */
    if (isUsedCUDA) {
        model.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
        model.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
    } else {
        model.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        model.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    }

    /**模型设置输入 */
    model.setInput(tensor4D);

    /**模型的前向传播 */
    cv::Mat outputTensor = model.forward();

    /**输出tensor的形状 */
    std::cout << "模型输出tensor的维度: " <<  outputTensor.dims << std::endl;
    std::cout << "模型输出tensor的形状: " <<  outputTensor.size << std::endl;
    
    return outputTensor;
}

void ImageClassification::postprocess(cv::Mat &tensor){
    if(tensor.empty()){
        throw std::invalid_argument("输入的Mat对象为空! ");
    }

    // 检查是否需要 Softmax
    bool needSoftmax = false;

    /** 连续的Mat对象[1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0] */
    /** 不连续的Mat的对象
     * [ 
     *   [1.0 2.0 3.0]
     *   [4.0 5.0 6.0]
     *   [7.0 8.0 9.0]
     * ]
     */
    if(tensor.isContinuous()){
        /**创建一个行指针 */
        float *pRow = tensor.ptr<float>(0);

        /**获取总元素个数 */
        int totalElements = tensor.rows * tensor.cols;

        for(int item = 0; item < totalElements; item++){
            if(*(pRow + item) > 1.0f){
                needSoftmax = true;
                break;
            }
        }

    } else{
        for(int row = 0; row < tensor.rows; row++){
            /**创建一个行指针*/
            float *pRow = tensor.ptr<float>(row);
            for(int col = 0; col < tensor.cols; col++){
                if(*(pRow + col) > 1.0f){
                    needSoftmax = true;
                    break;
                }
            }
            if(needSoftmax){
                break;
            }
        }
    }

    /**如果需要 Softmax，则进行归一化 */
    cv::Mat probabilities;
    if (needSoftmax) {
        probabilities = softmax(tensor.reshape(1, 1));
    } else {
        probabilities = tensor.reshape(1, 1);
    }

    /**输出tensor的形状 */
    std::cout << "probabilities的维度: " <<  probabilities.dims << std::endl;
    std::cout << "probabilities的形状: " <<  probabilities.size << std::endl;

    
    /**打印所有类别的概率 */
    std::cout << "\nAll class probabilities:" << std::endl;
    for (int i = 0; i < probabilities.cols; ++i) {
        std::cout << classLabel[i] << ": " << probabilities.at<float>(0, i) << std::endl;
    }

    /**获取最大置信度的类别索引 */
    cv::Point classIdPoint;
    double confidence;
    cv::minMaxLoc(probabilities, nullptr, &confidence, nullptr, &classIdPoint);

    /**输出预测结果  */
    int classId = classIdPoint.x;
    std::cout << "Predicted class: " << classLabel[classId] << ", Confidence: " << confidence << std::endl;
}

void ImageClassification::getNetModelInfo(cv::dnn::Net &netModel){
    /**获取网络模型所有层的名称 */
    std::vector<std::string> modelLayerName = netModel.getLayerNames();
    for(int i = 0; i < modelLayerName.size(); i++){
        std::cout << "第" << i << "层" << ": " << modelLayerName[i] << std::endl;
    }
    // https://blog.csdn.net/qq_35054151/article/details/112487691
}
