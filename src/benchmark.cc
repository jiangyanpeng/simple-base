#include "benchmark.h"
#include "log.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <time.h>

#ifdef _MSC_VER
#include <Winsock2.h>
#include <windows.h>
static int gettimeofday(struct timeval* tp, void* tzp) {
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;
    GetLocalTime(&wtm);
    tm.tm_year  = wtm.wYear - 1900U;
    tm.tm_mon   = wtm.wMonth - 1U;
    tm.tm_mday  = wtm.wDay;
    tm.tm_hour  = wtm.wHour;
    tm.tm_min   = wtm.wMinute;
    tm.tm_sec   = wtm.wSecond;
    tm.tm_isdst = -1;
    clock       = mktime(&tm);
    tp->tv_sec  = clock;
    tp->tv_usec = wtm.wMilliseconds * 1000UL;
    return (0);
}
#else
#include <sys/time.h>
#endif

uint64_t Timer::GetTimeUs() {
    timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000000UL + tv.tv_usec;
}

Item::Item(const std::string& target_name) {
    this->max_cost     = 0UL;
    this->min_cost     = std::numeric_limits<uint64_t>::max();
    this->avg_cost     = 0L;
    this->record_count = 0UL;
    this->enter        = 0UL;
    this->name         = target_name;
}

Benchmark::Benchmark(const uint32_t target_num, const std::vector<std::string>& target_name)
    : enable(false) {
    for (size_t target_idx = 0UL; target_idx < target_num; target_idx++) {
        this->item.push_back(target_name[target_idx]);
        this->name_list[target_name[target_idx]] = static_cast<unsigned int>(target_idx);
    }
}

void Benchmark::Init() {
    for (size_t target_idx = 0UL; target_idx < this->item.size(); target_idx++) {
        this->item[target_idx]                       = Item(this->item[target_idx].name);
        this->name_list[this->item[target_idx].name] = static_cast<unsigned int>(target_idx);
    }
}

void Benchmark::RecordExit(const uint32_t target) {
    if (!this->enable) {
        return;
    }
    uint64_t cur = Timer::GetTimeUs() - this->item[target].enter;
    Item& it     = this->item[target];
    it.record_count++;
    it.avg_cost +=
        (static_cast<int64_t>(cur) - it.avg_cost) / static_cast<int64_t>(it.record_count);
    it.max_cost = std::max(it.max_cost, cur);
    it.min_cost = std::min(it.min_cost, cur);
}

void Benchmark::Enable() {
    this->Init();
    this->enable = true;
}

void Benchmark::RecordEnter(const std::string& name) {
    if (this->name_list.end() == this->name_list.find(name)) {
        std::lock_guard<std::mutex> l(this->name_list_mutex_);
        this->item.push_back(name);
        this->name_list[name] = static_cast<uint32_t>(this->item.size() - 1);
    }
    auto& target = this->name_list[name];
    RecordEnter(target);
}
void Benchmark::RecordExit(const std::string& name) {
    auto& target = this->name_list[name];
    RecordExit(target);
}

void Benchmark::RecordEnter(const uint32_t target) {
    this->item[target].enter = Timer::GetTimeUs();
}

void Benchmark::Reset() {
    for (auto& benchdata : item) {
        benchdata.max_cost     = 0UL;
        benchdata.min_cost     = std::numeric_limits<uint64_t>::max();
        benchdata.avg_cost     = 0L;
        benchdata.record_count = 0UL;
        benchdata.enter        = 0UL;
    }
}

std::shared_ptr<Benchmark>&
BenchmarkInstance::CreateBenchmark(const uint32_t target_num,
                                   const std::vector<std::string>& target_name,
                                   const std::string& Benchmark_name) {
    if (target_name.size() != static_cast<size_t>(target_num)) {
        SIMPLE_LOG_ERROR(
            "BenchmarkInstance::CreateBenchmark faile, {}vs{}", target_name.size(), target_num);
    }

    BenchmarkInstance* instance = BenchmarkInstance::GetInstance();
    if (instance->benchmark_.find(Benchmark_name) != instance->benchmark_.end()) {
        SIMPLE_LOG_ERROR("BenchmarkInstance::CreateBenchmark faile");
    }
    instance->benchmark_[Benchmark_name] =
        std::shared_ptr<Benchmark>(new Benchmark(target_num, target_name));

    std::shared_ptr<Benchmark>& ret = instance->benchmark_[Benchmark_name];
    ret->Enable();
    return ret;
}

const std::map<std::string, std::shared_ptr<Benchmark>>& BenchmarkInstance::GetBenchmarks() {
    return BenchmarkInstance::GetInstance()->benchmark_;
}

void BenchmarkInstance::Reset() {
    auto& all_benchmarks = BenchmarkInstance::GetInstance()->benchmark_;
    for (auto& Benchmark : all_benchmarks) {
        if (Benchmark.second != nullptr) {
            Benchmark.second->Reset();
        }
    }
}

void BenchmarkInstance::GetReport(std::string file_prefix) {
    auto GetNowTime = []() -> std::string {
        char buf[30] = {0};
        time_t t     = time(nullptr);
        tm* local    = localtime(&t);
        strftime(buf, 30, "%Y-%m-%d-%H-%M-%S", local);
        return std::string(buf);
    };

    std::string file_name = file_prefix + "_benchmark_" + GetNowTime() + ".csv";
    std::ofstream save_file(file_name.c_str(), std::ios_base::trunc | std::ios_base::out);
    if (!save_file.is_open()) {
        SIMPLE_LOG_ERROR("{} path can't open", file_name.c_str());
        return;
    }

    save_file << "Target,Avg(ms),Max(ms),Min(ms),Times" << std::endl;
    auto& all_benchmarks = BenchmarkInstance::GetInstance()->benchmark_;
    for (auto& b : all_benchmarks) {
        auto& data = b.second->GetData();
        if (std::all_of(data.begin(), data.end(), [&](const Item& item) {
                return item.record_count == 0;
            })) {
            continue;
        }

        save_file << std::endl << std::endl << "Benchmark Name: " << b.first << std::endl;
        for (auto& r : data) {
            if (r.record_count == 0)
                continue;
            float min = (std::numeric_limits<uint64_t>::max() == r.min_cost)
                            ? 0.0f
                            : static_cast<float>(r.min_cost) / 1000.0f;
            float max = static_cast<float>(r.max_cost) / 1000.0f;
            float avg = static_cast<float>(r.avg_cost) / 1000.0f;
            save_file << r.name << "," << std::fixed << std::setprecision(3) << avg << ","
                      << std::fixed << std::setprecision(3) << max << "," << std::fixed
                      << std::setprecision(3) << min << "," << r.record_count << std::endl;
        }
    }
}

BenchmarkInstance* BenchmarkInstance::GetInstance() {
    static BenchmarkInstance* instance = new BenchmarkInstance();
    return instance;
}