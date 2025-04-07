#include "objectdetection.h"
#include <fstream>

// 定义并初始化静态成员变量
double ObjectDetection::modelNeedImgWidth = 640;
double ObjectDetection::modelNeedImgHeight = 640;

const float ObjectDetection::confidence_threshold = 0.25;


ObjectDetection::ObjectDetection(const std::string &modelPath, const std::string &classLabel, const std::string &imgPath)
:_label(classLabel), _model(modelPath), _imgPath(imgPath){
    
}


ObjectDetection::~ObjectDetection() {

}

void ObjectDetection::modelInfer(void) {
   
    /** 获取图像 */
    cv::Mat srcImg = readImg(this->_imgPath);

    /** 图像预处理 */
    cv::Mat processImg = imgPreprocess(srcImg);
    
    /** 获取图像的缩放因子 */
    scalingFactor = imgScalingFactor(processImg);
    

    /** 将图像转换为4维张量 */
    cv::Mat tensor = imgToTensor(processImg);

    // 获取张量的形状
    int batchSize = tensor.size[0];   // 批量大小
    int channels = tensor.size[1];   // 通道数
    int height = tensor.size[2];     // 高度
    int width = tensor.size[3];      // 宽度
    std::cout << "Tensor shape: " << batchSize << "x" << channels << "x" << height << "x" << width << std::endl;

    /** 获取标签信息 */
    std::vector<std::string> classNames = getClassNames();

    /** 获取模型 */
    cv::dnn::Net netModel = getNetModel();
    
    /** 模型设置设备 */
    setModelDevices(netModel);

    startTime = cv::getTickCount();
    
    /** 将4维度张量作为模型的输出，并且执行前向传播 */
    cv::Mat outTennsor = modelForward(netModel, tensor);
    
    /** 模型输出解析 */
    modelOutputProcess(outTennsor);
    
    /** 非极大值抑制处理*/
    nonMaxSuppressionProcess(srcImg, classNames);
    printf("9999\r\n");
    /** 输出显示 */
}

std::vector<std::string> ObjectDetection::getClassNames(void) {
    std::ifstream fp(this->_label);
    if(!fp.is_open()){
        throw std::runtime_error("打开文件失败！"); // 抛出异常
    }

    std::string className;
    std::vector<std::string> labelNames;

    while(!fp.eof()){
        std::getline(fp, className);
        if(className.length()){
            labelNames.push_back(className);
        }
    }
    fp.close();
    return labelNames;
}

cv::dnn::Net ObjectDetection::getNetModel(void){
    cv::dnn::Net net = cv::dnn::readNetFromONNX(this->_model);
    return net;
}

void ObjectDetection::setModelDevices(cv::dnn::Net &model){
    /**指定模型在cpu还是GPU */
    model.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    model.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
}

cv::Mat ObjectDetection::modelForward(cv::dnn::Net &model, cv::Mat &tensor){
    if(model.empty()){
        throw std::invalid_argument("模型为空！");
    }

    /** 设置网络输入 */
    model.setInput(tensor);

    /** 模型的前向传播 */
    cv::Mat outTensor = model.forward();
    return outTensor;
}

cv::Mat ObjectDetection::readImg(const std::string &imgPath){
    if(imgPath.empty()){
        throw std::invalid_argument("图像路径为空！");
    }

    cv::Mat srcImg;
    srcImg = cv::imread(imgPath, cv::IMREAD_COLOR);
    if(srcImg.empty()){
        throw std::invalid_argument("读取图像失败！"); 
    }
    return srcImg;
}

std::map<std::string, int> ObjectDetection::getImgWidthAndHeight(cv::Mat &srcImg){
    if(srcImg.empty()){
        throw std::invalid_argument("传递图像为空!");  
    }

    std::map<std::string, int> srcImgSize;
    srcImgSize["width"] = srcImg.cols;
    srcImgSize["heigth"] = srcImg.rows;
    srcImgSize["channel"] = srcImg.channels();
    return srcImgSize;
}

cv::Mat ObjectDetection::imgPreprocess(cv::Mat &srcImg){
    if(srcImg.empty()){
        throw std::invalid_argument("传递图像为空!");  
    }

    /** 创建一张正方形画布 */
    int _max = std::max(srcImg.cols, srcImg.rows);
    cv::Mat squareImg = cv::Mat::zeros(cv::Size(_max, _max), CV_8UC3);

    /** 创建一个感兴趣的区域 */
    cv::Rect roi(0, 0, srcImg.cols, srcImg.rows);

    /**将原图内容复制到正方形画布中 */
    srcImg.copyTo(squareImg(roi));
    return squareImg;
}

std::vector<float> ObjectDetection::imgScalingFactor(cv::Mat &srcImg){
    if(srcImg.empty()){
        throw std::invalid_argument("传递图像为空!");  
    }
    
    std::vector<float> scalingFactor;
    float x_factor = srcImg.cols / 640.0f;
	float y_factor = srcImg.rows / 640.0f;
    scalingFactor.push_back(x_factor);
    scalingFactor.push_back(y_factor);
    return scalingFactor;
}

cv::Mat ObjectDetection::imgToTensor(cv::Mat &srcImg){
    if(srcImg.empty()){
        throw std::invalid_argument("传递图像为空!");  
    }

    cv::Mat mTensor;

    /**指定缩放因子, 用于将图像像素值归一化到 [0, 1] 的范围 */
    double scalefactor = 1.0 / 255.0;

    /**目标图像的尺寸 */
    cv::Size dstImgSize = cv::Size(ObjectDetection::modelNeedImgHeight, ObjectDetection::modelNeedImgWidth);
    
    /** 做均值减法的量（此处不做均值减法） */
    cv::Scalar value = cv::Scalar(0, 0, 0);

    mTensor = cv::dnn::blobFromImage(srcImg, scalefactor, dstImgSize, value, true, false);
    return mTensor;
}


void ObjectDetection::modelOutputProcess(cv::Mat &outTensor){
    if(outTensor.empty()){
        throw std::invalid_argument("传递数据为空!");  
    }

    /** 解析模型输出 */
    /** 模型输出是一个 3D 张量（例如形状为 [1, 25200, 85]）*/
    /** 3D.size[1] 是检测框的数量（例如 25200）, 将此处类比于图像的高度 */
    /** preds.size[2] 是每个检测框的特征数（例如 85：4 个边界框坐标 + 1 个置信度 + 80 个类别得分）， 将此处类比于图像的宽度*/

    // 获取检测框数量和特征数
    int numDetections = outTensor.size[1]; // 检测框的数量
    int numFeatures = outTensor.size[2];  // 每个检测框的特征数

    /**创建一张类比图像 */
    /**
     * [
     *   [(x, y, cw, cH, c), 概率值i]
     *   [(x, y, cw, cH, c), 概率值i]
     *   [(x, y, cw, cH, c),概率值i]
     *   .....
     * ]
     */
    cv::Mat analogImage(outTensor.size[1], outTensor.size[2], CV_32F, outTensor.ptr<float>());

    /**遍历检查框 */
    for(int rowItem = 0; rowItem < analogImage.rows; rowItem++){

        /**获取置信度 */
        float confidence = *(analogImage.ptr<float>(rowItem) + 4);

		// std::cout << "第" << rowItem  << "行" << confidence << std::endl;
        /** 置信度过滤 */
        if(confidence < ObjectDetection::confidence_threshold){
            /** 该ROI舍弃，不做处理 */
            continue;
        }

        /**获取一整行数据 */
        cv::Mat analogImage_Row = analogImage.row(rowItem);

        /** 获取提取当前检测框的类别得分（第 5 列及之后的所有列） */
        cv::Mat allClassProbabilities = analogImage_Row.colRange(5, outTensor.size[2]);

        /** 找到类别得分中的最大值及其对应的类别索引 */
        double classProbability;            // 最大概率值
        cv::Point classIdIndex;             // 最大得分对应的类别索引
        cv::minMaxLoc(allClassProbabilities, 0, &classProbability, 0, &classIdIndex);

        /**类别概率*/
        // std::cout << classProbability << std::endl;


        /** 如果类别概率值大于0.25 */
        if(classProbability > 0.25){
            /**获取ROI信息 */
            float roi_cx = *(analogImage.ptr<float>(rowItem) + 0);
            float roi_cy = *(analogImage.ptr<float>(rowItem) + 1);
            float roi_ow = *(analogImage.ptr<float>(rowItem) + 2);
            float roi_oh = *(analogImage.ptr<float>(rowItem) + 3);

            /** 将模型输出后的ROI坐标变换到原始图像的坐标区域中 */
            int x = static_cast<int>((roi_cx - roi_ow / 2) * scalingFactor[0]);
            int y = static_cast<int>((roi_cy - roi_oh / 2) * scalingFactor[1]);
            int width = static_cast<int>(roi_ow * scalingFactor[0]);
			int height = static_cast<int>(roi_oh * scalingFactor[1]);

            // std::cout << x<< y << width << height<<std::endl;

            /** 准备区域 */
            cv::Rect box;
			box.x = x;
			box.y = y;
			box.width = width;
			box.height = height;

            /** 保存检测结果 */
			this->_detectionResult.classProbabilitieOfMax.push_back(classProbability);
            this->_detectionResult.ROIs.push_back(box);
			this->_detectionResult.maxClassProbabilitieOfIndex.push_back(classIdIndex.x);
        }
    }

}

void ObjectDetection::nonMaxSuppressionProcess(cv::Mat &srcImg, std::vector<std::string> &class_names){

    /** 颜色表*/
	std::vector<cv::Scalar> colors;
	colors.push_back(cv::Scalar(0, 255, 0));
	colors.push_back(cv::Scalar(0, 255, 255));
	colors.push_back(cv::Scalar(255, 255, 0));
	colors.push_back(cv::Scalar(255, 0, 0));
	colors.push_back(cv::Scalar(0, 0, 255));

    /** */
    std::vector<int> indexes;
    cv::dnn::NMSBoxes(this->_detectionResult.ROIs, this->_detectionResult.classProbabilitieOfMax, 0.25, 0.25, indexes);
    for (size_t i = 0; i < indexes.size(); i++) {
		int index = indexes[i];
		int idx = this->_detectionResult.maxClassProbabilitieOfIndex[index];
		cv::rectangle(srcImg, this->_detectionResult.ROIs[index], colors[idx % 5], 2, 8);
		cv::rectangle(srcImg, cv::Point(this->_detectionResult.ROIs[index].tl().x, this->_detectionResult.ROIs[index].tl().y - 20),
        cv::Point(this->_detectionResult.ROIs[index].br().x, this->_detectionResult.ROIs[index].tl().y), cv::Scalar(255, 255, 255), -1);
		cv::putText(srcImg, class_names[idx], cv::Point(this->_detectionResult.ROIs[index].tl().x, this->_detectionResult.ROIs[index].tl().y - 10), cv::FONT_HERSHEY_SIMPLEX, .5, cv::Scalar(0, 0, 0));
	}

    float t = (cv::getTickCount() - startTime) / static_cast<float>(cv::getTickFrequency());
	putText(srcImg, cv::format("FPS: %.2f", 1.0 / t), cv::Point(20, 40), cv::FONT_HERSHEY_PLAIN, 2.0, cv::Scalar(255, 0, 0), 2, 8);
	cv::imshow("OpenCV4.5.4 + YOLOv5", srcImg);
}
