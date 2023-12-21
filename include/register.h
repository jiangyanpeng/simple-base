#ifndef SIMPLE_BASE_REGISTER_H_
#define SIMPLE_BASE_REGISTER_H_

#include "common.h"
#include "log.h"

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

/// @brief Universal Register Base Singleton Class
/// @tparam Base
/// @note
/// The object created from the registrar is not the same object
template <typename Base>
class RegisterBase {
public:
    using Creator     = std::function<std::shared_ptr<Base>()>;
    using RegisterMap = std::unordered_map<std::string, Creator>;

    static RegisterBase& GetInstance() {
        static RegisterBase<Base> factory;
        return factory;
    }

    void Register(const std::string& key, Creator creator) {
        SIMPLE_LOG_DEBUG("will register %s creator\n", key.c_str());
        if (creator_map_.count(key)) {
            SIMPLE_LOG_WARN("type %s already registered\n", key.c_str());
            return;
        }
        creator_map_[key] = creator;
        return;
    }

    std::shared_ptr<Base> Create(const std::string& key) {
        SIMPLE_LOG_DEBUG("will create %s creator\n", key.c_str());
        if (!creator_map_.count(key)) {
            SIMPLE_LOG_ERROR("type %s not register\n", key.c_str());
            return nullptr;
        }
        return creator_map_[key]();
    }

private:
    RegisterMap creator_map_;
};

template <typename Base>
class CommonRegister {
public:
    using Creator = std::function<std::shared_ptr<Base>()>;
    CommonRegister(const std::string& type, Creator creator) {
        RegisterBase<Base>::GetInstance().Register(type, creator);
    }
};

#define REGISTER_COMMON_ENGINE(namespaces, key, classbase, classname)                   \
    namespace {                                                                         \
        std::shared_ptr<namespaces::classbase> creator_##key##_##classname() {          \
            return std::shared_ptr<namespaces::classbase>(new namespaces::classname()); \
        }                                                                               \
        static CommonRegister<namespaces::classbase>                                    \
            g_creator_##key##_##classname(#key, creator_##key##_##classname);           \
    }

#endif // SIMPLE_BASE_REGISTER_H_