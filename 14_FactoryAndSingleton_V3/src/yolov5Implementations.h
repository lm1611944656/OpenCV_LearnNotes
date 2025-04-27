#ifndef YOLOV5_IMPLEMENTATIONS_H
#define YOLOV5_IMPLEMENTATIONS_H

#include "factoryPattern.h"
#include "yolov5_interface.h"

// Libtorch 实现
class YOLOv5_Libtorch : public YOLOv5 {
public:
    void init(const std::string model_path, const Device_Type device_type, Model_Type model_type) override;
protected:
    void pre_process() override;
    void process() override;
};

// ONNXRuntime 实现
class YOLOv5_ONNXRuntime : public YOLOv5 {
public:
    void init(const std::string model_path, const Device_Type device_type, Model_Type model_type) override;
protected:
    void pre_process() override;
    void process() override;
};

// OpenCV 实现
class YOLOv5_OpenCV : public YOLOv5 {
public:
    void init(const std::string model_path, const Device_Type device_type, Model_Type model_type) override;
protected:
    void pre_process() override;
    void process() override;
};

// OpenVINO 实现
class YOLOv5_OpenVINO : public YOLOv5 {
public:
    void init(const std::string model_path, const Device_Type device_type, Model_Type model_type) override;
protected:
    void pre_process() override;
    void process() override;
};

// TensorRT 实现
class YOLOv5_TensorRT : public YOLOv5 {
public:
    void init(const std::string model_path, const Device_Type device_type, Model_Type model_type) override;
protected:
    void pre_process() override;
    void process() override;
};

// 使用宏注册算法
REGISTER_ALGO(YOLOv5, Algo_Type, Algo_Type::Libtorch, YOLOv5_Libtorch);
REGISTER_ALGO(YOLOv5, Algo_Type, Algo_Type::ONNXRuntime, YOLOv5_ONNXRuntime);
REGISTER_ALGO(YOLOv5, Algo_Type, Algo_Type::OpenCV, YOLOv5_OpenCV);
REGISTER_ALGO(YOLOv5, Algo_Type, Algo_Type::OpenVINO, YOLOv5_OpenVINO);
REGISTER_ALGO(YOLOv5, Algo_Type, Algo_Type::TensorRT, YOLOv5_TensorRT);

#endif // YOLOV5_IMPLEMENTATIONS_H