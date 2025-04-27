#ifndef YOLOV5_INTERFACE
#define YOLOV5_INTERFACE

#include <opencv2/opencv.hpp>

/**算法类型 */
enum Algo_Type {
    Libtorch,
    ONNXRuntime,
    OpenCV,
    OpenVINO,
    TensorRT,
};

/**设备类型 */
enum Device_Type {
    CPU,
    GPU,
};

/**模型类型 */
enum Model_Type {
    FP32,
    FP16,
    INT8,
};

class YOLOv5 {
public:

    /**定义一个纯虚函数(纯虚函数无需在基类中实现，但是必须在派生类中实现) */
    virtual void init(const std::string model_path, const Device_Type device_type, Model_Type model_type) = 0;

    void infer(const std::string image_path);

    virtual void release() {}

protected:
    virtual void pre_process() = 0;
    virtual void process() = 0;
    virtual void post_process();

    cv::Mat m_image;
    cv::Mat m_result;
    float* m_outputs_host;
};

#endif