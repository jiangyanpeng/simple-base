#include "image/image.h"
#include "log.h"
#include "manager/data_manager.h"
#include "register.h"

#include <iostream>

int main() {
    base::DataManager manager;
    void* data = manager.Malloc(100);

    set_level(Loger::INFO);
    SIMPLE_LOG_INFO("manage_data: %p\n", data);
    SIMPLE_LOG_INFO("hello world\n");
    SIMPLE_LOG_DEBUG("hello world\n");
    SIMPLE_LOG_WARN("hello world\n");
    SIMPLE_LOG_ERROR("hello world\n");
    auto op = RegisterBase<base::Image>::GetInstance().Create("resize");
    return 0;
}