#include "manager/data_manager.h"

#include "image/image.h"
#include "log.h"
#include "register.h"

#include <iostream>

int main() {
    base::DataManager manager;
    void* data = manager.Malloc(100);
    SIMPLE_LOG_INFO("hello world");
    SIMPLE_LOG_DEBUG("hello world");
    SIMPLE_LOG_WARN("hello world");
    SIMPLE_LOG_ERROR("hello world");
    SIMPLE_LOG_TRACE("hello world");
    auto op = RegisterBase<base::Image>::GetInstance().Create("resize");
    return 0;
}