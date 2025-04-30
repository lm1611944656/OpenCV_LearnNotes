/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: imagesegmentation.cpp
*   软件模块: 应用模块
*   版 本 号: 1.0
*   生成日期: 2025-04-16
*   作    者: lium
*   功    能: 图像分割
* 
*************************************************************************/

#include "imagesegmentation.h"
#include <fstream>


const int   ImgSegmentation::INPUT_IMG_WIDTH      = 640;   // 输入图像宽度
const int   ImgSegmentation::INPUT_IMG_HEIGHT     = 640;   // 输入图像高度
const float ImgSegmentation::SCORE_THRESHOLD      = 0.5;   // 类别得分阈值，用于过滤低置信度预测
const float ImgSegmentation::NMS_THRESHOLD        = 0.45;  // 非极大值抑制（NMS）的交并比（IoU）阈值
const float ImgSegmentation::CONFIDENCE_THRESHOLD = 0.45;  // 置信度阈值，用于过滤低置信度框

ImgSegmentation::ImgSegmentation(const std::string &modelPath, const std::string &labelPath, const std::string &imgPath)
:_modelPath(modelPath), _labelPath(labelPath), _imgPath(imgPath)
{
    deinit();
}

ImgSegmentation::~ImgSegmentation(){

}

void ImgSegmentation::deinit(void){
    classLabel.clear();
    modelOutTensor.clear();
}

void ImgSegmentation::imageInferenceDetection(void){
    /**读取类别标签 */
    readLabel();

    /**读取图像 */
    cv::Mat srcImg = readImg(this->_imgPath);

    /**图像预处理 */
    cv::Vec4d params;
    cv::Mat tensor4D = pre_process(srcImg, params);

    /**模型推理 */
    modelInference(tensor4D, false);

    /**模型后处理 */
    cv::Mat segResultImg = post_process(srcImg, this->modelOutTensor, this->classLabel, params);
    cv::namedWindow("segResultImg", cv::WINDOW_AUTOSIZE);
    cv::imshow("segResultImg", segResultImg);
}

/**视频推理检测 */
void ImgSegmentation::videoInferenceDetection(const std::string &videoPath)
{
    if (videoPath.empty()) {
        throw std::invalid_argument("传递参数错误: 视频路径为空!");
    }

    /**读取类别标签 */
    readLabel();

    /**打开视频文件 */
    cv::VideoCapture cap(videoPath);
    if (!cap.isOpened()) {
        throw std::runtime_error("无法打开视频文件: " + videoPath);
    }

    /**获取视频的基本信息 */
    int frameWidth = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int frameHeight = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    double fps = cap.get(cv::CAP_PROP_FPS);
    int totalFrames = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));

    /**创建输出视频文件（可选） */
    cv::VideoWriter outputVideo;
    bool saveOutput = true; // 设置为 false 如果不需要保存输出视频
    if (saveOutput) {
        std::string outputPath = "output_video.avi";
        outputVideo.open(outputPath, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, cv::Size(frameWidth, frameHeight));
        if (!outputVideo.isOpened()) {
            throw std::runtime_error("无法创建输出视频文件: " + outputPath);
        }
    }

    /**初始化窗口 */
    cv::namedWindow("Video Segmentation", cv::WINDOW_AUTOSIZE);

    /**逐帧处理视频 */
    cv::Mat frame;
    int frameCount = 0;
    while (cap.read(frame)) {
        frameCount++;
        std::cout << "Processing frame " << frameCount << " / " << totalFrames << std::endl;

        /**图像预处理 */
        cv::Vec4d params;
        cv::Mat tensor4D = pre_process(frame, params);

        /**模型推理 */
        modelInference(tensor4D, false); // 假设使用 CPU 推理

        /**模型后处理 */
        cv::Mat segResultImg = post_process(frame, this->modelOutTensor, this->classLabel, params);

        /**显示结果 */
        cv::imshow("Video Segmentation", segResultImg);

        /**保存结果到输出视频 */
        if (saveOutput) {
            outputVideo.write(segResultImg);
        }

        /**按 'q' 键退出 */
        if (cv::waitKey(1) == 'q') {
            break;
        }
    }

    /**释放资源 */
    cap.release();
    if (saveOutput) {
        outputVideo.release();
    }
    cv::destroyAllWindows();

}

void ImgSegmentation::readLabel(void){
    if(this->_labelPath.empty()){
        throw std::invalid_argument("传递参数错误!");
    }

    std::ifstream fp(_labelPath);
    if (!fp.is_open())
	{
		printf("could not open file...\n");
		exit(-1);
	}

    std::string className;
    while(!fp.eof()){
        std::getline(fp, className);
        if(className.length()){
            this->classLabel.push_back(className);
        }
    }

    /**关闭文件指针 */
    fp.close();
}

cv::Mat ImgSegmentation::readImg(const std::string &imgPath){
    if(imgPath.empty()){
        throw std::invalid_argument("传递参数错误!");
    }

    cv::Mat srcImg = cv::imread(imgPath, cv::IMREAD_COLOR);
    return srcImg;
}

void ImgSegmentation::imageBorderPadding(        
    const cv::Mat& srcImg,      
    cv::Mat& dstImg,
    cv::Vec4d& params,      // [ratio_x, ratio_y, dw, dh]
    const cv::Size& newShape,
    bool autoShape,     
    bool scaleFill,
    bool scaleUp,
    int stride,
    const cv::Scalar& color)
{
    if(srcImg.empty()){
        throw std::invalid_argument("输入参数错误!");
    }

    /**u获取原始图像尺寸 */
    cv::Size shape = srcImg.size(); 

    /**计算缩放比例 */
    float r = std::min((float)newShape.height / (float)shape.height, (float)newShape.width / (float)shape.width); 
    
    /**如果不允许放大，则限制缩放比例不超过1 */
    if (!scaleUp) {
        r = std::min(r, 1.0f); 
    }

    /**缩放比例 */
    float ratio[2]{ r, r }; 

    /**新尺寸 */
    int new_un_pad[2] = { (int)std::round((float)shape.width * r), (int)std::round((float)shape.height * r) };

    /**高度和宽度填充 */
    auto dw = (float)(newShape.width - new_un_pad[0]); 
    auto dh = (float)(newShape.height - new_un_pad[1]); 

    /**自定填充还是强制填充 */
    if (autoShape) { 
        dw = (float)((int)dw % stride);
        dh = (float)((int)dh % stride);
    } else if (scaleFill) { 
        dw = 0.0f;
        dh = 0.0f;
        new_un_pad[0] = newShape.width;
        new_un_pad[1] = newShape.height;
        ratio[0] = (float)newShape.width / (float)shape.width;
        ratio[1] = (float)newShape.height / (float)shape.height;
    }

    dw /= 2.0f; 
    dh /= 2.0f; 

    /**调整图像尺寸 */
    if (shape.width != new_un_pad[0] && shape.height != new_un_pad[1]) {
        cv::resize(srcImg, dstImg, cv::Size(new_un_pad[0], new_un_pad[1]));
    } else {
        dstImg = srcImg.clone();
    }
        
    /**上下、左右填充 */
    int top = int(std::round(dh - 0.1f)); 
    int bottom = int(std::round(dh + 0.1f)); 
    int left = int(std::round(dw - 0.1f)); 
    int right = int(std::round(dw + 0.1f));
    params[0] = ratio[0]; 
    params[1] = ratio[1];
    params[2] = left;
    params[3] = top;
    cv::copyMakeBorder(dstImg, dstImg, top, bottom, left, right, cv::BORDER_CONSTANT, color); 
}

cv::Mat ImgSegmentation::pre_process(cv::Mat &srcImg, cv::Vec4d &paddingParam){
    if(srcImg.empty()){
        throw std::invalid_argument("传递参数错误!");
    }

    cv::Mat tensor4D;   // [batch, channel, W, H]
    cv::Mat paddingImg;
    imageBorderPadding(srcImg, paddingImg, paddingParam, cv::Size(ImgSegmentation::INPUT_IMG_WIDTH, ImgSegmentation::INPUT_IMG_HEIGHT));       
    
    /**模型需要的参数 */
    float scale = 1.0 / 255.;                           // 将像素值缩放到[0,1]
    cv::Size inputSize = cv::Size(ImgSegmentation::INPUT_IMG_WIDTH, ImgSegmentation::INPUT_IMG_HEIGHT);            // 假设模型期望的输入大小为224x224
    cv::Scalar mean = cv::Scalar(0.485, 0.456, 0.406);  // 减去的均值（对于ResNet等模型）
    bool swapRB = true;                                 // 若模型使用RGB格式，则需交换R和B通道

    cv::dnn::blobFromImage(paddingImg, tensor4D, scale, inputSize, cv::Scalar(), swapRB, false); 
    return tensor4D;
}

void ImgSegmentation::modelInference(cv::Mat &tensor4D, bool isUseCUDA){
    if(tensor4D.empty()){
        throw std::invalid_argument("传递参数错误!");
    }

    /**读取网络模型 */
    cv::dnn::Net model = cv::dnn::readNet(this->_modelPath);
    if (model.empty()) {
        throw std::runtime_error("无法加载模型文件: " + _modelPath);
    }

    /** 判断是是GPU和CPU */
    if (isUseCUDA) {
        model.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
        model.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
    } else {
        model.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        model.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    }

    /**模型设置输入 */
    model.setInput(tensor4D);

    /**根据算子图指定输出层名称 */
    std::vector<std::string> output_layer_names{ "output0", "output1" }; 

    model.forward(this->modelOutTensor, output_layer_names); // 执行前向传播，获取网络输出
}

void ImgSegmentation::generateMask(
    const cv::Mat& maskProposals, 
    const cv::Mat& mask_protos, 
    TOutputSeg& output, 
    const TMaskParams& maskParams)
{
    /**获取结构体信息 */
    int seg_channels = maskParams.segChannels; // 分割通道数（掩膜特征维度）
    int net_width = maskParams.netWidth;       // 网络输入宽度
    int seg_width = maskParams.segWidth;       // 分割掩膜宽度
    int net_height = maskParams.netHeight;     // 网络输入高度
    int seg_height = maskParams.segHeight;     // 分割掩膜高度
    float mask_threshold = maskParams.maskThreshold; // 掩膜二值化阈值
    cv::Vec4f params = maskParams.params;      // LetterBox参数 [ratio_x, ratio_y, dw, dh]
    cv::Size src_img_shape = maskParams.srcImgShape; // 原始图像尺寸
    cv::Rect temp_rect = output.box;          // 当前检测框的矩形区域

    // 计算裁剪范围：将检测框映射到分割掩膜的空间
    int rang_x = floor((temp_rect.x * params[0] + params[2]) / net_width * seg_width);
    int rang_y = floor((temp_rect.y * params[1] + params[3]) / net_height * seg_height);
    int rang_w = ceil(((temp_rect.x + temp_rect.width) * params[0] + params[2]) / net_width * seg_width) - rang_x;
    int rang_h = ceil(((temp_rect.y + temp_rect.height) * params[1] + params[3]) / net_height * seg_height) - rang_y;

    rang_w = MAX(rang_w, 1); // 确保裁剪宽度至少为1
    rang_h = MAX(rang_h, 1); // 确保裁剪高度至少为1
    if (rang_x + rang_w > seg_width) { // 如果裁剪范围超出掩膜宽度，调整范围
        if (seg_width - rang_x > 0)
            rang_w = seg_width - rang_x;
        else
            rang_x -= 1;
    }
    if (rang_y + rang_h > seg_height) { // 如果裁剪范围超出掩膜高度，调整范围
        if (seg_height - rang_y > 0)
            rang_h = seg_height - rang_y;
        else
            rang_y -= 1;
    }

    // 定义裁剪范围：指定在掩膜原型中的裁剪区域
    std::vector<cv::Range> roi_rangs;
    roi_rangs.push_back(cv::Range(0, 1)); // 第一个维度固定为0
    roi_rangs.push_back(cv::Range::all()); // 第二个维度全选
    roi_rangs.push_back(cv::Range(rang_y, rang_h + rang_y)); // 高度范围
    roi_rangs.push_back(cv::Range(rang_x, rang_w + rang_x)); // 宽度范围

    // 裁剪掩膜原型：从掩膜原型中提取与当前检测框相关的部分
    cv::Mat temp_mask_protos = mask_protos(roi_rangs).clone();
    cv::Mat protos = temp_mask_protos.reshape(0, { seg_channels, rang_w * rang_h }); // 调整形状以便后续矩阵运算
    cv::Mat matmul_res = (maskProposals * protos).t(); // 矩阵乘法：生成具体的掩膜特征
    cv::Mat masks_feature = matmul_res.reshape(1, { rang_h, rang_w }); // 调整形状为二维掩膜

    cv::Mat dest, mask;

    // Sigmoid激活函数：将掩膜特征转换为概率值
    cv::exp(-masks_feature, dest); // 计算 exp(-x)
    dest = 1.0 / (1.0 + dest); // 应用 Sigmoid 激活函数：1 / (1 + exp(-x))

    // 将掩膜映射回原始图像坐标系：计算掩膜在原始图像中的位置
    int left = floor((net_width / seg_width * rang_x - params[2]) / params[0]);
    int top = floor((net_height / seg_height * rang_y - params[3]) / params[1]);
    int width = ceil(net_width / seg_width * rang_w / params[0]); // 映射后的宽度
    int height = ceil(net_height / seg_height * rang_h / params[1]); // 映射后的高度

    cv::resize(dest, mask, cv::Size(width, height), cv::INTER_NEAREST); // 调整掩膜大小以匹配原始图像
    mask = mask(temp_rect - cv::Point(left, top)) > mask_threshold; // 应用阈值，生成二值化掩膜

    output.boxMask = mask; // 将生成的掩膜存储到输出结构体中
}

void ImgSegmentation::plotResult(cv::Mat &srcImg, std::vector<TOutputSeg> result, std::vector<std::string> class_name){
    std::vector<cv::Scalar> color; // 随机颜色
    srand(time(0));
    for (int i = 0; i < class_name.size(); i++)
        color.push_back(cv::Scalar(rand() % 256, rand() % 256, rand() % 256));

    cv::Mat mask = srcImg.clone(); // 复制图像用于绘制掩膜
    for (int i = 0; i < result.size(); i++) {
        cv::rectangle(srcImg, result[i].box, cv::Scalar(255, 0, 0), 2); // 绘制检测框
        mask(result[i].box).setTo(color[result[i].id], result[i].boxMask); // 绘制掩膜
        std::string label = class_name[result[i].id] + ":" + cv::format("%.2f", result[i].confidence); // 标签
        int baseLine;
        cv::Size label_size = cv::getTextSize(label, 0.8, 0.8, 1, &baseLine); // 计算标签尺寸
        cv::putText(srcImg, label, cv::Point(result[i].box.x, result[i].box.y), cv::FONT_HERSHEY_SIMPLEX, 0.8, color[result[i].id], 1); // 绘制标签
    }
    addWeighted(srcImg, 0.5, mask, 0.5, 0, srcImg); // 合并图像和掩膜
}

cv::Mat ImgSegmentation::post_process(cv::Mat& image, std::vector<cv::Mat>& outputs, const std::vector<std::string>& class_name, cv::Vec4d& params){
    std::vector<int> class_ids;         // 类别ID列表
    std::vector<float> confidences;    // 置信度列表
    std::vector<cv::Rect> boxes;       // 检测框列表
    std::vector<std::vector<float>> picked_proposals; // 选中的掩膜提案

    float* data = (float*)outputs[0].data; // 获取网络输出数据

    const int dimensions = 117; // 每个检测框的特征维度（5+80+32）
    const int rows = 25200;     // 检测框总数
    for (int i = 0; i < rows; ++i) {
        float confidence = data[4]; // 置信度
        if (confidence >= CONFIDENCE_THRESHOLD) { // 过滤低置信度
            float* classes_scores = data + 5; // 类别得分
            cv::Mat scores(1, class_name.size(), CV_32FC1, classes_scores); // 类别得分矩阵
            cv::Point class_id;
            double max_class_score;
            cv::minMaxLoc(scores, 0, &max_class_score, 0, &class_id); // 获取最高得分类别
            if (max_class_score > SCORE_THRESHOLD) { // 过滤低得分
                float x = (data[0] - params[2]) / params[0]; // 转换坐标
                float y = (data[1] - params[3]) / params[1];
                float w = data[2] / params[0];
                float h = data[3] / params[1];
                int left = std::max(int(x - 0.5 * w), 0); // 左上角x
                int top = std::max(int(y - 0.5 * h), 0);  // 左上角y
                int width = int(w);                       // 宽度
                int height = int(h);                      // 高度
                boxes.push_back(cv::Rect(left, top, width, height)); // 添加检测框
                confidences.push_back(confidence); // 添加置信度
                class_ids.push_back(class_id.x);   // 添加类别ID

                std::vector<float> temp_proto(data + class_name.size() + 5, data + dimensions); // 提取掩膜提案
                picked_proposals.push_back(temp_proto);
            }
        }
        data += dimensions; // 移动指针到下一个检测框
    }

    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, SCORE_THRESHOLD, NMS_THRESHOLD, indices); // 非极大值抑制

    std::vector<TOutputSeg> output; // 最终输出结果
    std::vector<std::vector<float>> temp_mask_proposals;
    cv::Rect holeImgRect(0, 0, image.cols, image.rows); // 图像边界
    for (int i = 0; i < indices.size(); ++i) {
        int idx = indices[i];
        TOutputSeg result;
        result.id = class_ids[idx];          // 类别ID
        result.confidence = confidences[idx]; // 置信度
        result.box = boxes[idx] & holeImgRect; // 检测框与图像边界求交集
        temp_mask_proposals.push_back(picked_proposals[idx]); // 添加掩膜提案
        output.push_back(result);
    }

    TMaskParams mask_params;
    mask_params.params = params;          // LetterBox参数
    mask_params.srcImgShape = image.size(); // 原始图像尺寸
    for (int i = 0; i < temp_mask_proposals.size(); ++i) {
        generateMask(cv::Mat(temp_mask_proposals[i]).t(), outputs[1], output[i], mask_params); // 生成掩膜
    }

    plotResult(image, output, class_name); // 绘制结果

    return image;
}

/*************************************************************************
* 改动历史纪录：
Revision 1.0, 2025-04-10, lium
describe: 初始创建.
*************************************************************************/