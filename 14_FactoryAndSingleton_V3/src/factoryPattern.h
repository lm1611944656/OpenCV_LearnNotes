#ifndef FACTORY_PATTERN_H
#define FACTORY_PATTERN_H

#include <iostream>
#include <memory>
#include <map>
#include "singleton.h"

template<class BaseClass, class KeyType>
class AlgoFactory : public Singleton<AlgoFactory<BaseClass, KeyType>> {
public:
    // 定义创建函数类型
    //using CreateFunction = std::unique_ptr<BaseClass>(*)();
    typedef std::unique_ptr<BaseClass> (*CreateFunction)();

    /** 创建算法对象 */
    std::unique_ptr<BaseClass> create(const KeyType& algo_type) {
        auto it = m_algo_registry.find(algo_type);
        if (it == m_algo_registry.end()) {
            throw std::runtime_error("Algorithm type not registered!");
        }
        return it->second();
    }

    /** 注册算法 */
    void register_algo(const KeyType& algo_type, CreateFunction create_function) {
        m_algo_registry[algo_type] = create_function;
    }

private:
    /** 单例模式 */
    AlgoFactory() = default;
    friend class Singleton<AlgoFactory<BaseClass, KeyType>>;

    /** 算法注册表 */
    std::map<KeyType, CreateFunction> m_algo_registry;
};

// 定义一个宏来简化注册过程
#define REGISTER_ALGO(BaseClass, KeyType, algo_type, algo_class) \
    struct XYZ_##algo_class##_XYZ { \
        XYZ_##algo_class##_XYZ() { \
            AlgoFactory<BaseClass, KeyType>::GetInstance()->register_algo( \
                algo_type, \
                []() -> std::unique_ptr<BaseClass> { return std::make_unique<algo_class>(); } \
            ); \
        } \
    }; \
    static XYZ_##algo_class##_XYZ algo_class##_ABC;

#endif // FACTORY_PATTERN_H