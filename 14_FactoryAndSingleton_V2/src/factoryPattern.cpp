#include "factoryPattern.h"
#include <cassert>


// 注册算法方法
void AlgoFactory::register_algo(const Algo_Type& algo_type, CreateFunction create_function) {
    m_algo_registry[algo_type] = create_function;
}

// 创建算法对象方法
std::unique_ptr<YOLOv5> AlgoFactory::create(const Algo_Type& algo_type) {
    auto it = m_algo_registry.find(algo_type);
    if (it == m_algo_registry.end()) {
        throw std::runtime_error("Algorithm type not registered!");
    }
    return it->second();
}