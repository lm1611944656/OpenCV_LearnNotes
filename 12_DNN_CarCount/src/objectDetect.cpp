/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: objectDetect.h
*   软件模块: 应用模块
*   版 本 号: 1.0
*   生成日期: 2025-04-17
*   作    者: lium
*   功    能: 车辆计数(目标检测)
* 
*************************************************************************/

#include "objectDetect.h"
#include <fstream>

#define LINEWIDTH    1          // 绘制矩形框的线宽
#define LINELOCATION 500        // 撞线

const float CObjeectDetect::INPUT_IMG_WIDTH = 640.0F;
const float CObjeectDetect::INPUT_IMG_HEIGHT = 640.0F;
const float CObjeectDetect::SCORE_THRESHOLD = 0.5;              // 类别得分阈值，用于过滤低置信度预测(用于过滤矩阵的第5列~最后一列中低于该值的概率)
const float CObjeectDetect::NMS_THRESHOLD = 0.45;               // 过滤非极大值抑制（NMS）的交并比（IoU）阈值
const float CObjeectDetect::CONFIDENCE_THRESHOLD = 0.45;        // 置信度阈值，用于过滤低置信度框(过滤矩阵的第4列中低于该值的置信度罚值)

CObjeectDetect::CObjeectDetect(const std::string &modelPath, const std::string &labelPath)
:_modelPath(modelPath), _classLabelPath(labelPath)
{
    deinit();
}

CObjeectDetect::~CObjeectDetect()
{

}

void CObjeectDetect::deinit(void)
{
    _className.clear();
}

void CObjeectDetect::imageInference(const std::string &imgPath)
{
    /**读取类别标签 */
    readClassLabel();

    /**读取图像 */
    cv::Mat srcImg = readImg(imgPath);
    //drawLabel(srcImg, "Audi", 10, 10);
    //plotImg(srcImg, "srcImg");

    /**前处理 */
    cv::Mat tensor4D = pre_process(srcImg);
    //getMatInf(tensor4D);

    /**模型推理 */
    std::vector<cv::Mat> outTensor =  modelInference(tensor4D, CPU);

    /**后处理 */
    std::vector<cv::Rect> detections = post_process(outTensor, srcImg);
}

void CObjeectDetect::videoInference(const std::string &videoPath)
{
    if(videoPath.empty()){
        throw std::invalid_argument("传递参数错误!");
    }

    /**读取类别标签 */
    readClassLabel();

    /**捕获摄像头 */
    cv::Mat frame;
    cv::VideoCapture capture(videoPath);

    while(cv::waitKey(1) < 0){
        capture >> frame;
        if(frame.empty()){
            break;
        }
        cv::Mat srcImg = frame.clone();

        /**前处理 */
        cv::Mat tensor4D = pre_process(srcImg);

        /**模型推理 */
        std::vector<cv::Mat> outTensor =  modelInference(tensor4D, CPU);

        /**后处理 */
        std::vector<cv::Rect> detections = post_process(outTensor, srcImg);
    }

    /**释放视频捕获 */
    capture.release();
}

void CObjeectDetect::vehicleCounting(const std::string &videoPath){
    if(videoPath.empty()){
        throw std::invalid_argument("传递参数错误!");
    }

    /**一个视频中帧的数量 */
    int frameCount = 0;
    std::vector<cv::Point> detections_centers_old;      /**第0帧中所有检测框的中心点 */
	std::vector<cv::Point> detections_centers_new;      /**第1帧中所有检测框的中心点 */

    /**车辆的数量 */
    int carCount = 0;

    /**视频帧 */
    cv::Mat videoFrame;

    /**视频捕获 */
    cv::VideoCapture capture(videoPath);

    /**视频帧捕获(如果1ms内用户没有按下按键，返回 -1 ) */
    while(cv::waitKey(1) < 0){
        capture >> videoFrame;

        /**如果已经没有视频帧，直接退出捕获 */
        if(videoFrame.empty()){
            break;
        }

        /**复制一份视频帧，来做图像推理 */
        cv::Mat srcImg = videoFrame.clone();
        
        cv::Mat tensor4D = pre_process(srcImg);
       
        std::vector<cv::Mat> outTensor =  modelInference(tensor4D, CPU);

        std::vector<cv::Rect> detections = post_process(outTensor, srcImg, "car");

        /**是否是起使帧 */
        if(frameCount == 0){
            /**获取当前帧中所有目标框的中心点 */
            detections_centers_old = getCenterCoordinatesOfRectangle(detections);
        } else if(frameCount % 2 == 0){
            /**获取当前帧中所有目标框的中心点 */
            detections_centers_new = getCenterCoordinatesOfRectangle(detections);

            /**新的每一个点与旧的每一个点做距离计算，计算结果保留在二维矩阵中 */
            std::vector<std::vector<float>> distanceMatrix(
                detections_centers_new.size(),                      // 矩阵的行数量
                std::vector<float>(detections_centers_old.size())); // 矩阵的列数量

            for(int i = 0; i < detections_centers_new.size(); i++){
                for(int j = 0; j < detections_centers_old.size(); j++){
                    distanceMatrix[i][j] = euclideanDistance(detections_centers_old[j], detections_centers_new[i]);
                }
            }

            /**求取出二维矩阵中，每行矩形最小的点的索引 */
            std::vector<float> min_index(detections_centers_new.size());
            for(int i = 0; i < detections_centers_new.size(); i++){
                std::vector<float> distance_vevtor  = distanceMatrix[i];
                int index = std::min_element(distance_vevtor.begin(), distance_vevtor.end()) - distance_vevtor.begin();
                min_index[i] = index;
            }

            /**判断两个点是否和某一条直线相交 */
            // 遍历新中心点，判断是否跨越某条线
            for (size_t i = 0; i < detections_centers_new.size(); i++)
            {
                cv::Point p1 = detections_centers_new[i];  // 新中心点
                cv::Point p2 = detections_centers_old[min_index[i]];  // 对应的旧中心点

                // 打印新旧中心点
                // std::cout << p1 << " " << p2 << std::endl;

                // 判断两点是否跨越某条线（例如 y=500 的水平线）
                if (is_cross(p1, p2))
                {
                    // 如果跨越，则打印信息并增加计数器
                    std::cout << "is_cross" << p1 << " " << p2 << std::endl;
                    carCount++;
                }
            }
            // 更新旧中心点为当前帧的新中心点
            detections_centers_old = detections_centers_new;
        }

        /**视频帧数量递增 */
        frameCount++;

        // 在图像上绘制目标数量
        cv::putText(srcImg, "car num: " + std::to_string(carCount), cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 255), 1);

        // 绘制一条水平线（例如 y=500 的红线）
        cv::line(srcImg, cv::Point(0, LINELOCATION), cv::Point(1280, LINELOCATION), cv::Scalar(0, 0, 255));

        // 显示结果图像
        cv::imshow("output", srcImg);
    }
}

void CObjeectDetect::readClassLabel(void){
    if(this->_classLabelPath.empty()){
        throw std::invalid_argument("传递参数错误!");
    }

    /**创建一个文件指针 */
    std::ifstream fp(this->_classLabelPath);
    if(!fp.is_open()){
        throw std::runtime_error("打开文件失败！");
    }

    std::string className;
    while (!fp.eof())
    {
        std::getline(fp, className);
        if(!className.empty()){
            this->_className.push_back(className);
        }
    }
    fp.close();
}

cv::Mat CObjeectDetect::readImg(const std::string &imgPath){
    if(imgPath.empty()){
        throw std::invalid_argument("传递参数错误!");
    }
    cv::Mat srcImg = cv::imread(imgPath, cv::IMREAD_COLOR);
    return srcImg;
}

void CObjeectDetect::plotImg(cv::Mat &srcImg, const char *winName){
    if((srcImg.empty()) && (winName == NULL)){
        throw std::invalid_argument("传递参数错误！");
    }

    cv::namedWindow(winName, cv::WINDOW_AUTOSIZE);
    cv::imshow(winName, srcImg);
}

void CObjeectDetect::drawLabel(cv::Mat &srcImg, const std::string &drawText, int x_startPoint, int y_startPoint){
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

cv::Mat CObjeectDetect::pre_process(cv::Mat &srcImg){
    if(srcImg.empty()){
        throw std::invalid_argument("传递参数错误！");
    }

    /**数据预处理 */

    /**tensor变化 */
    cv::Mat tensor4D;
    cv::dnn::blobFromImage(srcImg, tensor4D, 1.0 / 255.0, cv::Size(CObjeectDetect::INPUT_IMG_WIDTH, CObjeectDetect::INPUT_IMG_HEIGHT), cv::Scalar(), true, false);
    return tensor4D;
}

std::vector<cv::Mat> CObjeectDetect::modelInference(cv::Mat &tensor4D, TDeviceType deviceType){
    if(tensor4D.empty()){
        throw std::invalid_argument("传递参数错误!");
    }

    /**根据算子图指定输出层名称 */
    std::vector<cv::Mat> outTensor;

    /**读取网络模型 */
    cv::dnn::Net model = cv::dnn::readNet(this->_modelPath);
    if (model.empty()) {
        throw std::runtime_error("无法加载模型文件: " + _modelPath);
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

std::vector<cv::Rect> CObjeectDetect::post_process(std::vector<cv::Mat> &modelOutResult, cv::Mat &srcImg){
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
    float x_factor = srcImg.cols / CObjeectDetect::INPUT_IMG_WIDTH;
    float y_factor = srcImg.rows / CObjeectDetect::INPUT_IMG_HEIGHT;

    //getMatInf(modelOutResult[0]);

    /**获取第1维度和第2维度的数据 */
    cv::Mat tensor2D(modelOutResult[0].size[1], modelOutResult[0].size[2], CV_32F, modelOutResult[0].ptr<float>());

    getMatInf(tensor2D);

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
        if(confidence < CObjeectDetect::CONFIDENCE_THRESHOLD){
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
        
        if(maxClassProbabilityScores > CObjeectDetect::SCORE_THRESHOLD){
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

    std::vector<cv::Rect> m_boxes_NMS;

    cv::dnn::NMSBoxes(m_boxes, m_confidences, CObjeectDetect::SCORE_THRESHOLD, CObjeectDetect::NMS_THRESHOLD, indices);
	
    for (size_t i = 0; i < indices.size(); i++)
	{
        /**获取保留下来的框对应的索引 */
		int idx = indices[i];

        /**获取索引对应的矩形框 */
		cv::Rect box = m_boxes[idx];
		m_boxes_NMS.push_back(box);



		int left = box.x;
		int top = box.y;
		int width = box.width;
		int height = box.height;
		cv::rectangle(srcImg, cv::Point(left, top), cv::Point(left + width, top + height), color.at(COLOR_GREEN), LINEWIDTH);


		std::string label = cv::format("%.2f", m_confidences[idx]);
 
        /**获取类别名称 */
		label = _className[m_classIDs[idx]] + ":" + label;

        /**直接将标签固定 */
		// label = "car";
		drawLabel(srcImg, label, left, top);
	}
    
    /**是否要显示图像 */
    cv::imshow("OpenCV4.5.4 + YOLOv5", srcImg);
    return m_boxes_NMS;
}

std::vector<cv::Rect> CObjeectDetect::post_process(std::vector<cv::Mat> &modelOutResult, cv::Mat &srcImg, const std::string &className){
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
    float x_factor = srcImg.cols / CObjeectDetect::INPUT_IMG_WIDTH;
    float y_factor = srcImg.rows / CObjeectDetect::INPUT_IMG_HEIGHT;

    //getMatInf(modelOutResult[0]);

    /**获取第1维度和第2维度的数据 */
    cv::Mat tensor2D(modelOutResult[0].size[1], modelOutResult[0].size[2], CV_32F, modelOutResult[0].ptr<float>());

    getMatInf(tensor2D);
    std::cout << __FILE__ << "  " << __LINE__ << std::endl;

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
        if(confidence < CObjeectDetect::CONFIDENCE_THRESHOLD){
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
        
        if(maxClassProbabilityScores > CObjeectDetect::SCORE_THRESHOLD){
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

    std::vector<cv::Rect> m_boxes_NMS;

    cv::dnn::NMSBoxes(m_boxes, m_confidences, CObjeectDetect::SCORE_THRESHOLD, CObjeectDetect::NMS_THRESHOLD, indices);
	
    for (size_t i = 0; i < indices.size(); i++)
	{
        /**获取保留下来的框对应的索引 */
		int idx = indices[i];

        /**获取索引对应的矩形框 */
		cv::Rect box = m_boxes[idx];
		m_boxes_NMS.push_back(box);

		int left = box.x;
		int top = box.y;
		int width = box.width;
		int height = box.height;
		cv::rectangle(srcImg, cv::Point(left, top), cv::Point(left + width, top + height), color.at(COLOR_GREEN), LINEWIDTH);

		std::string label = cv::format("%.2f", m_confidences[idx]);
 
        /**获取类别名称 */
		// label = _className[m_classIDs[idx]] + ":" + label;

        /**直接将标签固定 */
		label = className;
		drawLabel(srcImg, label, left, top);
	}
    
    /**是否要显示图像 */
    //cv::imshow("OpenCV4.5.4 + YOLOv5", srcImg);
    return m_boxes_NMS;
}

std::vector<cv::Point> CObjeectDetect::getCenterCoordinatesOfRectangle(std::vector<cv::Rect> &detections){
    if(detections.size() == 0){
        throw std::invalid_argument("传递参数错误!");
    }

    int detectionSize = detections.size();
    std::vector<cv::Point> rectCenterPoint(detectionSize);
    for(size_t i = 0; i < detectionSize; i++){
        rectCenterPoint[i] = cv::Point(detections[i].x + detections[i].width / 2, detections[i].y + detections[i].height / 2);
    }
    return rectCenterPoint;
}

float CObjeectDetect::euclideanDistance(cv::Point &startPoint, cv::Point &endPoint){
    return std::sqrt(std::pow(endPoint.x - startPoint.x, 2) + std::pow(endPoint.y - startPoint.y, 2));
}

bool CObjeectDetect::is_cross(cv::Point p1, cv::Point p2){
    // 方法一(不准确)
#if 0
    if (p1.x == p2.x) return false;

	int y = LINELOCATION;  //line1: y = 500
	float k = (p1.y - p2.y) / (p1.x - p2.x);  //
	float b = p1.y - k * p1.x; //line2: y = kx + b
	float x = (y - b) / k;
	return (x > std::min(p1.x, p2.x) && x < std::max(p1.x, p2.x));
#endif
    // 方法二(稍微准确)
    return (p1.y <= LINE_HEIGHT && p2.y > LINE_HEIGHT) || (p1.y > LINE_HEIGHT && p2.y <= LINE_HEIGHT);
}

void CObjeectDetect::getMatInf(cv::Mat &_mat){
    if(_mat.empty()){
        throw std::invalid_argument("传递参数错误！");
    }

    std::cout << "Mat的维度: " << _mat.dims << std::endl;
    std::cout << "Mat的size: " << _mat.size << std::endl;

    /**判断Mat的内存是否是连续的 */
    if (_mat.isContinuous()){
        std::cout << "Mat在内存中是连续的内存对象!" << std::endl;
    } else {
        std::cout << "Mat在内存中是不连续的内存对象!" << std::endl;
    }
}

void CObjeectDetect::getModelInf(cv::dnn::Net &model){
    if(model.empty()){
        throw std::invalid_argument("传递参数错误!");
    }

    /**从模型中获取每一层的信息 */
    std::vector<std::string> modelLayerInfo = model.getLayerNames();
    for(int i = 0; i < modelLayerInfo.size(); i++){
        std::cout << "第" << i << "层" << ": " << modelLayerInfo[i] << std::endl;
    }

    /**获取未连接层的所在索引 */
    std::vector<int> unconnectedOutLayersIndexs = model.getUnconnectedOutLayers();
    std::cout << "有多少个未被链接的层：" << unconnectedOutLayersIndexs.size() << std::endl;
    std::cout << "未被连接的层索引：" << unconnectedOutLayersIndexs[0] << std::endl;

    // 打印未连接输出层的名称
    std::cout << "未连接输出层的名称：" << std::endl;
    for (int idx : unconnectedOutLayersIndexs) {
        // 注意：OpenCV 的层索引从 1 开始，而 C++ 数组索引从 0 开始
        std::cout << modelLayerInfo[idx - 1] << std::endl;
    }
}

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-04-17, lium
* describe: 初始创建.
*************************************************************************/