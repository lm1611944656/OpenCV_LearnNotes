#ifndef YOLOV5_IMPLEMENTATIONS_H
#define YOLOV5_IMPLEMENTATIONS_H

#include "factoryPattern.h"

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

#endif // YOLOV5_IMPLEMENTATIONS_H