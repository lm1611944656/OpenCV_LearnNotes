#ifndef FACTORY_PATTERN_H
#define FACTORY_PATTERN_H

#include <iostream>
#include <fstream>
#include <memory>
#include <map>
#include <opencv2/opencv.hpp>

/**枚举出需要实现的算法 */
enum Algo_Type {
    Libtorch,
    ONNXRuntime,
    OpenCV,
    OpenVINO,
    TensorRT,
};

enum Device_Type {
    CPU,
    GPU,
};

enum Model_Type {
    FP32,
    FP16,
    INT8,
};

class YOLOv5 {
public:
    /**声明一个纯虚函数(纯虚函数需要在派生类中重写) */
    virtual void init(const std::string model_path, const Device_Type device_type, Model_Type model_type) = 0;

    /**声明普通函数 */
    void infer(const std::string image_path);

    /**声明一个普通虚函数 */
    virtual void release() {}

protected:
    /**声明一个纯虚函数(纯虚函数需要在派生类中重写) */
    virtual void pre_process() = 0;

    /**声明一个纯虚函数(纯虚函数需要在派生类中重写) */
    virtual void process() = 0;

    /**声明一个普通虚函数 */
    virtual void post_process();

    cv::Mat m_image;
    cv::Mat m_result;
    float* m_outputs_host;
};


/**在工程模式内部实现工程模式的单例模式 */
#if 0
class AlgoFactory {
public:
    typedef std::unique_ptr<YOLOv5> (*CreateFunction)();

    static AlgoFactory& instance();

    void register_algo(const Algo_Type& algo_type, CreateFunction create_function);
    std::unique_ptr<YOLOv5> create(const Algo_Type& algo_type);

private:
    AlgoFactory();
    std::map<Algo_Type, CreateFunction> m_algo_registry;
};
#endif


#include "singleton.h"

// 工厂类继承单例模式
class AlgoFactory : public Singleton<AlgoFactory> {

public:
    /**工厂要生产的算法 */
    std::unique_ptr<YOLOv5> create(const Algo_Type& algo_type); 

private:
    /**单例模式需要构造函数private */
    AlgoFactory(); 
    friend class Singleton<AlgoFactory>; 

private:
    //using CreateFunction = std::unique_ptr<YOLOv5>(*)();  // 类似的函数：YOLOv5 *func();
    typedef std::unique_ptr<YOLOv5> (*CreateFunction)();    // 类似的函数：YOLOv5 *func();

    /**算法注册表 */
    std::map<Algo_Type, CreateFunction> m_algo_registry;

    /**往注册表中注册算法 */
    void register_algo(const Algo_Type& algo_type, CreateFunction create_function); 
};

#endif // FACTORY_PATTERN_H