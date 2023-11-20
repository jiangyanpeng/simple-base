#ifndef SIMPLE_BASE_BENCHMARK_H_
#define SIMPLE_BASE_BENCHMARK_H_

#include "common.h"

#include <map>
#include <memory>
#include <mutex>
#include <stdint.h>
#include <vector>

class EXPORT_API Timer {
public:
    static uint64_t GetTimeUs();
};

class EXPORT_API Item {
public:
    uint64_t max_cost;
    uint64_t min_cost;
    int64_t avg_cost;

    uint64_t record_count;
    uint64_t enter;

    std::string name;

    Item(const std::string& target_name);
};

class EXPORT_API Benchmark {
public:
    Benchmark(const uint32_t target_num, const std::vector<std::string>& target_name);

    void Init();
    void Enable();
    void Disable() { this->enable = false; }
    void RecordEnter(const std::string& name);
    void RecordExit(const std::string& name);
    void RecordEnter(const uint32_t target);
    void RecordExit(const uint32_t target);

    const std::vector<Item>& GetData() { return this->item; }

    void Reset();

private:
    std::vector<Item> item;
    std::map<std::string, uint32_t> name_list;
    bool enable;
    std::mutex name_list_mutex_;
};

class EXPORT_API BenchmarkInstance {
public:
    static std::shared_ptr<Benchmark>& CreateBenchmark(const uint32_t target_num,
                                                       const std::vector<std::string>& target_name,
                                                       const std::string& Benchmark_name);

    static const std::map<std::string, std::shared_ptr<Benchmark>>& GetBenchmarks();
    static void Reset();
    static void GetReport(std::string file_prefix = "");

private:
    std::map<std::string, std::shared_ptr<Benchmark>> benchmark_;
    static BenchmarkInstance* GetInstance();
};

#define LOCAL_BENCHMARK_REG(...) \
    enum class _LocalBenchmarkTarget : uint32_t { __VA_ARGS__, LOCAL_BENCHMARK_TARGET_NUM }

#define LOCAL_BENCHMARK_NAME_REG(...) \
    static const std::vector<std::string> g_LocalBenchmarkTargetName = {__VA_ARGS__}

#define LOCAL_BENCHMARK_NAME_REG_WITH_NAME(...) \
    LOCAL_BENCHMARK_REG(__VA_ARGS__);           \
    LOCAL_BENCHMARK_NAME_REG(STRINGIFY(__VA_ARGS__))

#define LOCAL_BENCHMARK_NUM _LocalBenchmarkTarget::LOCAL_BENCHMARK_TARGET_NUM

#define LOCAL_BENCHMARK_CREATE(NAME)                                                          \
    static std::shared_ptr<BenchMark>& g_LocalBenchmark = BenchMarkInstance::CreateBenchmark( \
        static_cast<uint32_t>(LOCAL_BENCHMARK_NUM), g_LocalBenchmarkTargetName, #NAME)

#define LOCAL_BENCHMARK g_LocalBenchmark

#define LOCAL_BENCHMARK_RECORD_IN(BENCHMARK_TARGET) \
    LOCAL_BENCHMARK->RecordEnter(static_cast<uint32_t>(_LocalBenchmarkTarget::BENCHMARK_TARGET))
#define LOCAL_BENCHMARK_RECORD_OUT(BENCHMARK_TARGET) \
    LOCAL_BENCHMARK->RecordExit(static_cast<uint32_t>(_LocalBenchmarkTarget::BENCHMARK_TARGET))

#define LOCAL_BENCHMARK_RECORD_NAME_IN(BENCHMARK_NAME) LOCAL_BENCHMARK->RecordEnter(BENCHMARK_NAME)
#define LOCAL_BENCHMARK_RECORD_NAME_OUT(BENCHMARK_NAME) LOCAL_BENCHMARK->RecordExit(BENCHMARK_NAME)

/* Scope Recorder */
class BenchmarkScopeRecorder {
public:
    explicit BenchmarkScopeRecorder(const uint32_t target,
                                    std::shared_ptr<Benchmark>& local_benmark)
        : target_(target), local_benmark_(local_benmark) {
        this->local_benmark_->RecordEnter(target_);
    }
    ~BenchmarkScopeRecorder() { this->local_benmark_->RecordExit(target_); }

private:
    uint32_t target_;
    std::shared_ptr<Benchmark>& local_benmark_;
};

/* Scope Recorder With Name*/
class BenchmarkScopeNameRecorder {
public:
    explicit BenchmarkScopeNameRecorder(const std::string& name,
                                        std::shared_ptr<Benchmark>& local_benmark)
        : name_(name), local_benmark_(local_benmark) {
        this->local_benmark_->RecordEnter(name_);
    }
    ~BenchmarkScopeNameRecorder() { this->local_benmark_->RecordExit(name_); }

private:
    const std::string name_;
    std::shared_ptr<Benchmark>& local_benmark_;
};

#define LOCAL_BENCHMARK_SCOPE_RECORD(BENCHMARK_TARGET)     \
    BenchmarkScopeRecorder __benchmark_##BENCHMARK_TARGET( \
        static_cast<uint32_t>(_LocalBenchmarkTarget::BENCHMARK_TARGET), LOCAL_BENCHMARK)

#define LOCAL_BENCHMARK_SCOPE_NAME_RECORD(BENCHMARK_NAME) \
    BenchmarkScopeNameRecorder __benchmark__(BENCHMARK_NAME, LOCAL_BENCHMARK)

#endif // SIMPLE_BASE_BENCHMARK_H_