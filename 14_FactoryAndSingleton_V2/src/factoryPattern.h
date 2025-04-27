#ifndef FACTORY_PATTERN_H
#define FACTORY_PATTERN_H

#include <iostream>
#include <fstream>
#include <memory>
#include <map>
#include "singleton.h"
#include "yolov5_interface.h"

class AlgoFactory :public Singleton<AlgoFactory>{
public:
    //using CreateFunction = std::unique_ptr<YOLOv5>(*)();
    typedef std::unique_ptr<YOLOv5>(*CreateFunction)();

    /**创建算法对象 */
    std::unique_ptr<YOLOv5> create(const Algo_Type& algo_type); 
    
    /**注册算法 */
    void register_algo(const Algo_Type& algo_type, CreateFunction create_function); 

private:
    /**单例模式 */
    AlgoFactory() = default; // 私有构造函数
    friend class Singleton<AlgoFactory>; 

    /**算法注册表 */
    std::map<Algo_Type, CreateFunction> m_algo_registry; 
};
    

/**
 * @brief 定义一个宏来简化算法注册过程
 * @param algo_type 输入参数，需要注册的算法的类型
 * @param algo_class 输入参数，需要注册的算法的具体类
 * 
 * 宏定义中是一个结构体，结构体中是其自身的构造函数；
 * 构造函数中调用工厂类的注册函数
 * 并且宏定义中还实力化了一个结构体对象
 */
#define REGISTER_ALGO(algo_type, algo_class) \
    struct XYZ_##algo_class##_XYZ { \
        XYZ_##algo_class##_XYZ() { \
            AlgoFactory::GetInstance()->register_algo( \
                algo_type, \
                []() -> std::unique_ptr<YOLOv5> { return std::make_unique<algo_class>(); } \
            ); \
        } \
    }; \
    static XYZ_##algo_class##_XYZ algo_class##_ABC;

/**例如 */
/**
    struct XYZ_ONNXRuntimeAlgo_XYZ {
        XYZ_ONNXRuntimeAlgo_XYZ() {
            AlgoFactory::GetInstance()->register_algo(
                Algo_Type::ONNXRuntime,
                []() -> std::unique_ptr<YOLOv5> { return std::make_unique<ONNXRuntimeAlgo>(); }
            );
        }
    };
    static XYZ_ONNXRuntimeAlgo_XYZ ONNXRuntimeAlgo_ABC;
 */
    

#endif // FACTORY_PATTERN_H