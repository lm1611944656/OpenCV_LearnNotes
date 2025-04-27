#if 1
#include "factoryPattern.h"
#include "yolov5Implementations.h"

int main() {

    try {
        // 获取工厂单例实例（通过智能指针）
        std::shared_ptr<AlgoFactory> factory = AlgoFactory::GetInstance();

        // 创建不同类型的 YOLOv5 对象
        auto algo1 = factory->create(Algo_Type::Libtorch);
        algo1->init("model_path", Device_Type::GPU, Model_Type::FP32);
        algo1->infer("image_path");

        auto algo2 = factory->create(Algo_Type::ONNXRuntime);
        algo2->init("model_path", Device_Type::CPU, Model_Type::FP16);
        algo2->infer("image_path");

        // 尝试创建未注册的算法类型
        auto algo3 = factory->create(static_cast<Algo_Type>(99)); // 应该抛出异常
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    printf("****************************************************\r\n");

    try {
        // 获取工厂实例（通过裸指针）
        AlgoFactory* factory = AlgoFactory::GetInstancePtr();

        // 创建不同类型的 YOLOv5 对象
        auto algo1 = factory->create(Algo_Type::Libtorch);
        algo1->init("model_path", Device_Type::GPU, Model_Type::FP32);
        algo1->infer("image_path");

        auto algo2 = factory->create(Algo_Type::ONNXRuntime);
        algo2->init("model_path", Device_Type::CPU, Model_Type::FP16);
        algo2->infer("image_path");

        // 尝试创建未注册的算法类型
        auto algo3 = factory->create(static_cast<Algo_Type>(99)); // 应该抛出异常
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }


    return 0;
}

#endif

#if 0
#include <iostream>

class Base {
public:
    // 纯虚函数，必须在派生类中实现
    virtual void pureVirtualFunc() = 0;

    // 普通虚函数1，提供了默认实现
    virtual void virtualFunc1() {
        std::cout << "Base::virtualFunc1() called" << std::endl;
    }

    // 普通虚函数2，提供了默认实现
    virtual void virtualFunc2() {
        std::cout << "Base::virtualFunc2() called" << std::endl;
    }

    // 虚析构函数，确保正确释放资源
    virtual ~Base() {}
};

class Derived : public Base {
public:
    // 实现基类的纯虚函数
    void pureVirtualFunc() override {
        std::cout << "Derived::pureVirtualFunc() called" << std::endl;
    }

    // 重写虚函数1
    void virtualFunc1() override {
        std::cout << "Derived::virtualFunc1() called" << std::endl;
    }

    // 不重写虚函数2，直接继承基类的实现
};

int main() {
    Derived derivedObj;
    Base* basePtr = &derivedObj; // 基类指针指向派生类对象

    // 调用纯虚函数
    basePtr->pureVirtualFunc(); // 调用的是派生类的版本
    derivedObj.pureVirtualFunc(); // 调用的是派生类的版本

    // 调用虚函数1
    basePtr->virtualFunc1(); // 调用的是派生类的版本
    derivedObj.virtualFunc1(); // 调用的是派生类的版本

    // 调用虚函数2
    basePtr->virtualFunc2(); // 调用的是基类的版本
    derivedObj.virtualFunc2(); // 调用的是基类的版本

    return 0;
}
#endif