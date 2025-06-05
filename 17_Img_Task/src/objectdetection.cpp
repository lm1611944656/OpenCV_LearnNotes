/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: objectdetection.cpp
*   软件模块: 目标检测
*   版 本 号: 1.0
*   生成日期: 2025-05-29
*   作    者: lium
*   功    能: 图像检测源文件定义
* 
************************************************************************/

#include "objectdetection.h"
#include <fstream>
#include "logging.h"

#define LINEWIDTH    1          // 绘制矩形框的线宽

const float CObjDetection::INPUT_WIDTH = 640.0F;
const float CObjDetection::INPUT_HEIGHT = 640.0F;
const float CObjDetection::SCORE_THRESHOLD = 0.5;
const float CObjDetection::NMS_THRESHOLD = 0.5;
const float CObjDetection::CONFIDENCE_THRESHOLD = 0.5;

CObjDetection::CObjDetection(const std::string &modelFile, std::string &classFile)
:modelFile_(modelFile), classFile_(classFile)
{
    deInit();

    readClassLabel();
}

CObjDetection::~CObjDetection()
{

}

void CObjDetection::deInit(void)
{
    className_.clear();
}

std::vector<TDetectResult> CObjDetection::imgDetectTask(std::string &imgPath)
{
    /**读取图像 */
    cv::Mat srcImg = this->readImg(imgPath);

    /**预处理 */
    cv::Mat tensor4D = this->preprocess(srcImg);

    /**模型推理 */
    std::vector<cv::Mat> outTensor =  this->modelInference(tensor4D, CPU);

    /**后处理 */
    std::vector<TDetectResult> detections = this->postprocess(outTensor, srcImg);
    return detections;
}

void CObjDetection::readClassLabel(void)
{
    if(this->classFile_.empty()){
        NN_LOG_ERROR("传递参数错误!");
        throw std::invalid_argument("传递参数错误!");
    }

    /**创建一个文件指针 */
    std::ifstream fp(this->classFile_);
    if(!fp.is_open()){
        throw std::runtime_error("打开文件失败！");
    }

    std::string className;
    while (!fp.eof())
    {
        std::getline(fp, className);
        if(!className.empty()){
            this->className_.push_back(className);
        }
    }
    fp.close();
}

cv::Mat CObjDetection::readImg(const std::string &imgPath)
{
    if(imgPath.empty()){
        NN_LOG_ERROR("传递参数错误!");
        throw std::invalid_argument("传递参数错误!");
    }
    cv::Mat srcImg = cv::imread(imgPath, cv::IMREAD_COLOR);
    return srcImg;
}

cv::Mat CObjDetection::preprocess(cv::Mat &srcImg)
{
    if(srcImg.empty()){
        throw std::invalid_argument("传递参数错误！");
    }

    /**数据预处理 */

    /**tensor变化 */
    cv::Mat tensor4D;
    cv::dnn::blobFromImage(srcImg, tensor4D, 1.0 / 255.0, cv::Size(CObjDetection::INPUT_WIDTH, CObjDetection::INPUT_HEIGHT), cv::Scalar(), true, false);
    return tensor4D;
}

std::vector<cv::Mat> CObjDetection::modelInference(cv::Mat &tensor4D, TDeviceType deviceType)
{
    if(tensor4D.empty()){
        NN_LOG_ERROR("传递参数错误!");
        throw std::invalid_argument("传递参数错误!");
    }

    /**根据算子图指定输出层名称 */
    std::vector<cv::Mat> outTensor;

    /**读取网络模型 */
    cv::dnn::Net model = cv::dnn::readNet(this->modelFile_);
    if (model.empty()) {
        throw std::runtime_error("无法加载模型文件: " + modelFile_);
    }
    
    /**获取模型信息 */
    // getModelInf(model);

    /** 判断是是GPU和CPU */
    if (deviceType == GPU) {
        model.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
        model.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
    } else if(deviceType == CPU){
        model.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        model.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    }

    /**模型设置输入 */
    model.setInput(tensor4D);

    /**需要多个输出层 */
    //std::vector<std::string> output_layer_names{ "output0", "output1" }; 

    /**现在只有一个输出层 */
    std::vector<std::string> output_layer_names{"output0"};
    model.forward(outTensor, output_layer_names); 
    return outTensor;
}

std::vector<TDetectResult> CObjDetection::postprocess(std::vector<cv::Mat> &modelOutResult, cv::Mat &srcImg)
{
    if((modelOutResult.size() == 0) && (srcImg.empty())){
        throw std::invalid_argument("传递参数错误!");
    }

    /**满足要求的类别概率值及其位置 */
    // [x, y, w, h, c, (概率项)]
    std::vector<cv::Rect> m_boxes;      // 保存前4列满足要求数据
    std::vector<float> m_confidences;   // 保存第4列满足要求的类别置信度
    std::vector<int> m_classIDs;        // 保存满足要求的概率值所在位置


    /**计算缩放因子
     *             原始图像宽
     * x_factor = ----------
     *             目标图像宽
     * 
     *            原始图像高
     * y_factor = ----------
     *            目标图像高
     */
    float x_factor = srcImg.cols / CObjDetection::INPUT_WIDTH;
    float y_factor = srcImg.rows / CObjDetection::INPUT_HEIGHT;

    //getMatInf(modelOutResult[0]);

    /**获取第1维度和第2维度的数据 */
    cv::Mat tensor2D(modelOutResult[0].size[1], modelOutResult[0].size[2], CV_32F, modelOutResult[0].ptr<float>());

    //getMatInf(tensor2D);

    /**指针偏移量(获取类别概率的位置) */
    int offsetVal = 5;

    int tensor2DRowNum = tensor2D.rows;
    int tensor2DColNum = tensor2D.cols;

    /**类别得分的概率数量个数 */
    int classProbabilityCount = tensor2DColNum - offsetVal;

    for(int row = 0; row < tensor2DRowNum; row++){
        
        /**创建一个行指针 */
        float *pRow = tensor2D.ptr<float>(row);

        /**获取类别置信度 */
        float confidence = *(pRow + 4);

        /**判断类别置信度 */
        if(confidence < CObjDetection::CONFIDENCE_THRESHOLD){
            continue;
        }

         /** 创建一个新的类别概率矩阵 cv::Mat 来存储类别概率值 */
        cv::Mat classProbability(1, classProbabilityCount, CV_32F);

        /** 将第 5 列到最后一列的数据复制到 classProbability 中 */
        float *pOffset = pRow + offsetVal;
        std::memcpy(classProbability.ptr<float>(), pOffset, classProbabilityCount * sizeof(float));

        /** 示例：打印类别概率值 */
        // for (int c = 0; c < classProbabilityCount; c++) {
        //     std::cout << "Class " << c << " probability: " << classProbability.at<float>(0, c) << std::endl;
        // }

        /**在类别概率矩阵中寻找最大的类别概率，以及对应的索引 */
        cv::Point maxClassProbabilityLocation;      // 最大类别概率在类别概率矩阵中的位置
        double maxClassProbabilityScores;           // 最大的类别概率的值
        cv::minMaxLoc(classProbability, 0, &maxClassProbabilityScores, 0, &maxClassProbabilityLocation);
        
        if(maxClassProbabilityScores > CObjDetection::SCORE_THRESHOLD){
            /**保存位置和概率值 */
            m_confidences.push_back(confidence);
            m_classIDs.push_back(maxClassProbabilityLocation.x);

            /**获取模型推理后，生成的预测框 */
            float cx = *(pRow + 0);
            float cy = *(pRow + 1);
            float cw = *(pRow + 2);
            float ch = *(pRow + 3);

            /**将预测框的尺寸，反推回原始图像的尺寸上去 */
            int x = static_cast<int>((cx - cw / 2) * x_factor);
            int y = static_cast<int>((cy - ch / 2) * y_factor);
            int width = static_cast<int>(cw * x_factor);
            int height = static_cast<int>(ch * y_factor);

            /**保存矩形框 */
            m_boxes.push_back(cv::Rect(x, y, width, height));
        }
    }
    /*********************************NMS***************************************/
    /**
     *  // 假设我们有 5 个边界框及其对应的置信度分数
        std::vector<cv::Rect> m_boxes = {
            cv::Rect(100, 100, 50, 50),  // 左上角 (100, 100)，宽高 (50, 50)
            cv::Rect(110, 110, 50, 50),
            cv::Rect(200, 200, 60, 60),
            cv::Rect(210, 210, 60, 60),
            cv::Rect(300, 300, 70, 70)
        };
        std::vector<float> m_confidences = {0.9, 0.75, 0.8, 0.85, 0.6};
     */
    
    /**保存，最后满足要求的预测框所在概率矩阵的位置 */
    /** 保留的框索引: 0 2 3 */
    std::vector<int> indices;

    /**保存检查结果 */
    std::vector<TDetectResult> detectResultVector;
    detectResultVector.clear();

    cv::dnn::NMSBoxes(m_boxes, m_confidences, CObjDetection::SCORE_THRESHOLD, CObjDetection::NMS_THRESHOLD, indices);
	
    for (size_t i = 0; i < indices.size(); i++)
	{
        /**获取保留下来的框对应的索引 */
		int idx = indices[i];

        /**获取索引对应的矩形框 */
		cv::Rect box = m_boxes[idx];

        /**在检测到的目标类别上绘制矩形框 */
		// int left = box.x;
		// int top = box.y;
		// int width = box.width;
		// int height = box.height;
		//cv::rectangle(srcImg, cv::Point(left, top), cv::Point(left + width, top + height), color.at(COLOR_GREEN), LINEWIDTH);

        /**获取类别概率 */
		std::string probability = cv::format("%.2f", m_confidences[idx]);
 
        /**获取类别名称 */
		std::string label = className_[m_classIDs[idx]] + ":" + probability;

        /**在检测到的目标类别上绘制类别名称*/
		//drawLabel(srcImg, label, left, top);

        TDetectResult detectResult;
        detectResult.box = box;
        detectResult.className = className_[m_classIDs[idx]];
        detectResult.probability = probability;
        detectResultVector.push_back(detectResult);
	}
    
    /**是否要显示图像 */
    //cv::imshow("OpenCV4.5.4 + YOLOv5", srcImg);

    return detectResultVector;
}

void CObjDetection::drawLabel(cv::Mat &srcImg, const std::string &drawText, int x_startPoint, int y_startPoint)
{
    /**需要绘制文本的基线到文本底部的距离 */
    int baseLine;

    /**测量需要绘制文本的宽高 */
    /**
     * drawText: 需要绘制的文本
     * cv::FONT_HERSHEY_SIMPLEX 字体的大小
     * 0.7 字体的粗细
     * 1 线条类型
     * baseLine 输出参数，用于存储文本基线位置
     */
    cv::Size drawTextSize =  cv::getTextSize(drawText, cv::FONT_HERSHEY_SIMPLEX, 0.7, 1, &baseLine);

    /**如果 y_startPoint的值小于文本高度，则将其设置文本高度  */
    y_startPoint = std::max(y_startPoint, drawTextSize.height);

    /**定义矩形区域的左上角坐标点 */
    cv::Point rectLeftTopPoint = cv::Point(x_startPoint, y_startPoint + drawTextSize.height);
    cv::Point rectRightBottonPonin = cv::Point(x_startPoint + drawTextSize.width + 300, baseLine + drawTextSize.height + y_startPoint + 300);

    /**绘制矩形框 */
    //cv::rectangle(srcImg, rectLeftTopPoint, rectRightBottonPonin, color.at(COLOR_YELLOW), 1);

    /**绘制文本 */
    cv::putText(srcImg, drawText, cv::Point(x_startPoint, y_startPoint + drawTextSize.height), cv::FONT_HERSHEY_SIMPLEX, 0.7, color.at(COLOR_RED), 1);
}

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-05-29, lium
* describe: 初始创建.
*************************************************************************/