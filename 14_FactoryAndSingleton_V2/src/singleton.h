#ifndef SINGLETON_H
#define SINGLETON_H

#include <memory>
#include <mutex>

template <typename T>
class Singleton{

    /*1. 单例模式的构造函数不允许外部访问
      2. 声明为protected目的是子类继承单例模式类时，可以通过子类构造基类*/
protected:
    Singleton() = default;

    // 单例模式中允许拷贝构造
    Singleton(const Singleton<T> &) = delete;

    // 单例模式中不允许赋值构造
    Singleton& operator = (const Singleton<T> &st) = delete;

    // 只希望单例模式会返回有且仅有一个实例对象，并且不希望手动回收单例对象的实例
    // 所以就定义为一个智能指针的单例实例
    // 使用静态的目的是：当想使用Singleton时，所有的对象只有一个_instance;
    static std::shared_ptr<T> _instance;

public:
    // 方式1：返回单例模式（智能指针）
    static std::shared_ptr<T> GetInstance(){
        // 进入该函数先标记变量(该变量默认是false, 一旦执行了该语句就会被执行为true)
        // 设置为static目的是只是初始化一次就好了(初始化后一直为true)
        static std::once_flag s_flag;

        // 检测变量是否是true,如果是执行lambda表达式
        std::call_once(s_flag, [&](){
            _instance = std::shared_ptr<T> (new T);
        });

        return _instance;
    }

    // 方式2：返回单例模式（裸指针）
    static T* GetInstancePtr() {
        return GetInstance().get(); // 调用 GetInstance() 并返回其裸指针
    }

    ~Singleton(){
        std::cout<< "this is singleton destruct!"<<std::endl;
    }

    // 打印地址
    void PrintAddress(){
        std::cout << _instance.get() << std::endl;
    }
};

// 一个类的static变量一定要定义的。
// static变量在类的头文件中只能声明，不能定义。
// 如果类不是模版类，在头文件中声明的static变量，需要在源文件中的全局域定义(一定要在源文件中全局域定义)
// 如果类是模版类，在头文件中声明的static变量，需要在头文件中的全局域定义

template <typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;

#endif // SINGLETON_H