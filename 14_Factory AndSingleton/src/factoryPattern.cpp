#include "factoryPattern.h"
#include <cassert>
#include "yolov5Implementations.h"

// YOLOv5 的 infer 方法实现
void YOLOv5::infer(const std::string image_path) {
    // m_image = cv::imread(image_path);
    // if (m_image.empty()) {
    //     std::cerr << "Error: Unable to load image!" << std::endl;
    //     return;
    // }
    std::cout << "读取图像成功：" << image_path << std::endl;
    pre_process();
    process();
    post_process();
}

// YOLOv5 的 post_process 方法实现
void YOLOv5::post_process() {
    std::cout << "Post-processing..." << std::endl;
}


/**在工程模式内部实现工程模式的单例模式 */
#if 0
AlgoFactory::AlgoFactory() {
    register_algo(Algo_Type::Libtorch, []() -> std::unique_ptr<YOLOv5> { return std::make_unique<YOLOv5_Libtorch>(); });
    register_algo(Algo_Type::ONNXRuntime, []() -> std::unique_ptr<YOLOv5> { return std::make_unique<YOLOv5_ONNXRuntime>(); });
    register_algo(Algo_Type::OpenCV, []() -> std::unique_ptr<YOLOv5> { return std::make_unique<YOLOv5_OpenCV>(); });
    register_algo(Algo_Type::OpenVINO, []() -> std::unique_ptr<YOLOv5> { return std::make_unique<YOLOv5_OpenVINO>(); });
    register_algo(Algo_Type::TensorRT, []() -> std::unique_ptr<YOLOv5> { return std::make_unique<YOLOv5_TensorRT>(); });
}

// 工厂单例实例方法
AlgoFactory& AlgoFactory::instance() {
    static AlgoFactory algo_factory;
    return algo_factory;
}

// 注册算法方法
void AlgoFactory::register_algo(const Algo_Type& algo_type, CreateFunction create_function) {
    m_algo_registry[algo_type] = create_function;
}

// 创建算法对象方法
std::unique_ptr<YOLOv5> AlgoFactory::create(const Algo_Type& algo_type) {
    assert(("algo type not exists!", m_algo_registry.find(algo_type) != m_algo_registry.end()));
    return m_algo_registry[algo_type]();
}

#endif

AlgoFactory::AlgoFactory() {

    /**手动注册所有算法（硬编码注册算法） */
    register_algo(Algo_Type::Libtorch, []() -> std::unique_ptr<YOLOv5> { return std::make_unique<YOLOv5_Libtorch>(); });
    register_algo(Algo_Type::ONNXRuntime, []() -> std::unique_ptr<YOLOv5> { return std::make_unique<YOLOv5_ONNXRuntime>(); });
    register_algo(Algo_Type::OpenCV, []() -> std::unique_ptr<YOLOv5> { return std::make_unique<YOLOv5_OpenCV>(); });
    register_algo(Algo_Type::OpenVINO, []() -> std::unique_ptr<YOLOv5> { return std::make_unique<YOLOv5_OpenVINO>(); });
    register_algo(Algo_Type::TensorRT, []() -> std::unique_ptr<YOLOv5> { return std::make_unique<YOLOv5_TensorRT>(); });
}

// 注册算法方法
void AlgoFactory::register_algo(const Algo_Type& algo_type, CreateFunction create_function) {
    m_algo_registry[algo_type] = create_function;
}

// 创建算法对象方法
std::unique_ptr<YOLOv5> AlgoFactory::create(const Algo_Type& algo_type) {
    
    /**查表法，查找需要创建的算法 */
    auto it = m_algo_registry.find(algo_type);
    if (it == m_algo_registry.end()) {
        throw std::runtime_error("Algorithm type not registered!");
    }
    return it->second();
}

