#include "manager/data_manager.h"

#include "log.h"
#include <iostream>

#define CONFIG_SDK_LICENSE_ENABLE_LOG 1
int main() {
    std::cout << "hello world" << std::endl;
    base::DataManger manager;
    void* data = manager.Malloc(100);
    std::cout << "data: " << data << std::endl;
    SIMPLE_LOG_INFO("hello world");
    return 0;
}