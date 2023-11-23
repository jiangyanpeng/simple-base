#ifndef SIMPLE_BASE_PIPE_MANAGER_H_
#define SIMPLE_BASE_PIPE_MANAGER_H_

#include "common.h"
#include "log.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

#define THREADPOOL_MAX_NUM 16
//#define  THREADPOOL_AUTO_GROW

/// @brief Thread pool
/// which can submit anonymous function executions of variable parameter
/// functions or lambda expressions, obtain execution return values, do not directly support class
/// member functions, support class static member functions or global functions, operator()
/// functions, etc
/// @jiangyanpeng https://github.com/jiangyanpeng/threadpool
namespace base {
class EXPORT_API PipeManager {
public:
    inline PipeManager(unsigned short size = 4) { AddThread(size); }
    inline ~PipeManager() {
        run_ = false;
        task_cv_.notify_all();
        for (auto& thread : pool_) {
            // thread.detach();
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

public:
    /// @brief Submit a task and call. get() to obtain the return value. It will wait for the task
    /// to complete and obtain the return value
    /// @note
    /// There are two ways to call class members
    /// bind：.Commit(std::bind(&A::sayHello, &a));
    /// mem_fn：.Commit(std::mem_fn(&A::sayHello), this)
    template <class F, class... Args>
    auto Commit(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        if (!run_) {
            // throw runtime_error("commit on task, But thread pool is stopped.");
            SIMPLE_LOG_ERROR("commit on task, But thread pool is stopped");
            return {};
        }
        // 1. get the return value type of f function
        // 2. bind the function and package the parameters of the function
        // 3. push task/function in container
        // typename std::result_of<F(Args...)>::type;
        using RetType = decltype(f(args...));
        auto task     = std::make_shared<std::packaged_task<RetType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<RetType> future = task->get_future();
        {
            std::lock_guard<std::mutex> lock{lock_};
            tasks_.emplace([task]() { (*task)(); });
        }
#ifdef THREADPOOL_AUTO_GROW
        if (idl_thread_num_ < 1 && pool_.size() < THREADPOOL_MAX_NUM)
            AddThread(1);
#endif
        task_cv_.notify_one();

        return future;
    }

    /// @brief Set Thread stop
    void Stop() { run_ = false; }

    /// @brief Get number of idle threads
    int GetIdlCount() const { return idl_thread_num_; }

    /// @brief Get All threads
    std::vector<std::thread> GetThread() const { return pool_; }

    /// @brief Get number of threads
    int GetThreadCount() const { return pool_.size(); }

#ifndef THREADPOOL_AUTO_GROW
private:
#endif

    /// @brief Workflow of worker threads
    /// 1. get a pending task
    /// 2. if the queue is considered empty, wait until there is a task
    /// 3. get a task from the queue based on first in, first out
    /// 4. run task
    void AddThread(unsigned short size) {
        for (; pool_.size() < THREADPOOL_MAX_NUM && size > 0; --size) {
            pool_.emplace_back([this] {
                while (run_) {
                    Task task;
                    {
                        std::unique_lock<std::mutex> lock{lock_};
                        task_cv_.wait(lock, [this] { return !run_ || !tasks_.empty(); });
                        if (!run_ && tasks_.empty())
                            return;
                        task = move(tasks_.front());
                        tasks_.pop();
                    }
                    idl_thread_num_--;
                    task();
                    idl_thread_num_++;
                }
            });
            idl_thread_num_++;
        }
    }

private:
    using Task = std::function<void()>;  // defined task types
    std::vector<std::thread> pool_;      // thread pool
    std::queue<Task> tasks_;             // task queue
    std::mutex lock_;                    // synchronous lock
    std::condition_variable task_cv_;    // conditional blocking
    std::atomic<bool> run_{true};        // is the thread pool work
    std::atomic<int> idl_thread_num_{0}; // number of idle threads
};

} // namespace base
#endif // SIMPLE_BASE_PIPE_MANAGER_H_