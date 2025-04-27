#include "yolov5Implementations.h"
#include <iostream>


// YOLOv5_Libtorch 实现
void YOLOv5_Libtorch::init(const std::string model_path, const Device_Type device_type, Model_Type model_type) {
    std::cout << "Initializing Libtorch YOLOv5 model..." << std::endl;
}

void YOLOv5_Libtorch::pre_process() {
    std::cout << "Libtorch pre-processing..." << std::endl;
}

void YOLOv5_Libtorch::process() {
    std::cout << "Libtorch processing..." << std::endl;
}

// YOLOv5_ONNXRuntime 实现
void YOLOv5_ONNXRuntime::init(const std::string model_path, const Device_Type device_type, Model_Type model_type) {
    std::cout << "Initializing ONNXRuntime YOLOv5 model..." << std::endl;
}

void YOLOv5_ONNXRuntime::pre_process() {
    std::cout << "ONNXRuntime pre-processing..." << std::endl;
}

void YOLOv5_ONNXRuntime::process() {
    std::cout << "ONNXRuntime processing..." << std::endl;
}

// YOLOv5_OpenCV 实现
void YOLOv5_OpenCV::init(const std::string model_path, const Device_Type device_type, Model_Type model_type) {
    std::cout << "Initializing OpenCV YOLOv5 model..." << std::endl;
}

void YOLOv5_OpenCV::pre_process() {
    std::cout << "OpenCV pre-processing..." << std::endl;
}

void YOLOv5_OpenCV::process() {
    std::cout << "OpenCV processing..." << std::endl;
}

// YOLOv5_OpenVINO 实现
void YOLOv5_OpenVINO::init(const std::string model_path, const Device_Type device_type, Model_Type model_type) {
    std::cout << "Initializing OpenVINO YOLOv5 model..." << std::endl;
}

void YOLOv5_OpenVINO::pre_process() {
    std::cout << "OpenVINO pre-processing..." << std::endl;
}

void YOLOv5_OpenVINO::process() {
    std::cout << "OpenVINO processing..." << std::endl;
}

// YOLOv5_TensorRT 实现
void YOLOv5_TensorRT::init(const std::string model_path, const Device_Type device_type, Model_Type model_type) {
    std::cout << "Initializing TensorRT YOLOv5 model..." << std::endl;
}

void YOLOv5_TensorRT::pre_process() {
    std::cout << "TensorRT pre-processing..." << std::endl;
}

void YOLOv5_TensorRT::process() {
    std::cout << "TensorRT processing..." << std::endl;
}