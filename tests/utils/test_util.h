#ifndef SIMPLE_BASE_TEST_UTILS_H_
#define SIMPLE_BASE_TEST_UTILS_H_

#include <random>
#include <string>
#include <vector>

template <typename T>
int init_random(T* host_data, int n, T range_min, T range_max) {
    static std::mt19937 g(42);
    std::uniform_real_distribution<> rnd(range_min, range_max);

    for (int i = 0; i < n; i++) {
        host_data[i] = static_cast<T>(rnd(g));
    }

    return 0;
}

int read_binary_file(const std::string& file_path, void* buffer, long length);

int write_binary_file(const std::string& file_path, void* buffer, long length);

#endif // SIMPLE_BASE_TEST_UTILS_H_