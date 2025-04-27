#include "factoryPattern.h"
#include "yolov5Implementations.h"

int main() {
    try {
        // 获取工厂实例
        auto factory = AlgoFactory<YOLOv5, Algo_Type>::GetInstance();

        // 创建 Libtorch 对象
        auto algo_libtorch = factory->create(Algo_Type::Libtorch);
        algo_libtorch->init("path/to/model", Device_Type::GPU, Model_Type::FP32);

        // 创建 ONNXRuntime 对象
        auto algo_onnx = factory->create(Algo_Type::ONNXRuntime);
        algo_onnx->init("path/to/model", Device_Type::CPU, Model_Type::FP16);
    
        // 创建 ONNXRuntime 对象
        auto algo_onnx2 = factory->create(Algo_Type::ONNXRuntime);
        algo_onnx2->init("path/to/model", Device_Type::CPU, Model_Type::FP16);
    
        
        // 尝试创建未注册的算法类型
        auto algo3 = factory->create(static_cast<Algo_Type>(99)); // 应该抛出异常
        algo3->init("path/to/model", Device_Type::CPU, Model_Type::FP16);
    
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}