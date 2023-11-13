#include "manager/data_manager.h"

#include <iostream>

int main() {
    std::cout << "hello world" << std::endl;
    base::DataManger manager;
    void* data = manager.Malloc(100);
    std::cout << "data: " << data << std::endl; 
    return 0;
}