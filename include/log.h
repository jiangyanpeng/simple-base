#ifndef SIMPLE_BASE_LOG_H_
#define SIMPLE_BASE_LOG_H_

#ifdef CONFIG_SIMPLE_BASE_ENABLE_SPDLOG
#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>
namespace simple_logger {
class Logger {
private:
    std::shared_ptr<spdlog::logger> console_logger;

    Logger() {
        try {
            this->console_logger = spdlog::stdout_color_mt("console");
            spdlog::set_level(spdlog::level::debug);
            spdlog::set_pattern("[%p-%t %Y-%m-%d %H:%M:%S.%e.%f] [%L] %v");
        } catch (const spdlog::spdlog_ex& ex) {
            std::cerr << "spdlog::spdlog create faile" << std::endl;
            exit(-1);
        }
    }

    Logger(Logger const&) = delete;
    void operator=(Logger const&) = delete;

public:
    ~Logger() {}

    static Logger& get_logger() {
        static Logger new_loger;
        return new_loger;
    }

    void log(std::string a_str) { this->console_logger->debug("{}", a_str); }

    void set_level(spdlog::level::level_enum log_level) {
        this->console_logger->set_level(log_level);
    }

    template <typename... Args>
    void debug(const char* fmt, const Args... args) {
        this->console_logger->debug(fmt, args...);
    }

    template <typename... Args>
    void info(const char* fmt, const Args... args) {
        this->console_logger->info(fmt, args...);
    }

    template <typename... Args>
    void trace(const char* fmt, const Args... args) {
        this->console_logger->trace(fmt, args...);
    }

    template <typename... Args>
    void warn(const char* fmt, const Args... args) {
        this->console_logger->warn(fmt, args...);
    }

    template <typename... Args>
    void error(const char* fmt, const Args... args) {
        this->console_logger->error(fmt, args...);
    }

    template <typename... Args>
    void critical(const char* fmt, const Args... args) {
        this->console_logger->critical(fmt, args...);
        exit(-1);
    }
};
} // namespace simple_logger

static simple_logger::Logger& global_logger = simple_logger::Logger::get_logger();

#include <string.h>
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define SIMPLE_LOG_DEBUG(...)                               \
    global_logger.debug("[{}:{}]", __FILENAME__, __LINE__); \
    global_logger.debug(__VA_ARGS__)

#define SIMPLE_LOG_INFO(...) global_logger.info(__VA_ARGS__)
#define SIMPLE_LOG_TRACE(...) global_logger.trace(__VA_ARGS__)
#define SIMPLE_LOG_WARN(...) global_logger.warn(__VA_ARGS__)
#define SIMPLE_LOG_ERROR(...)                               \
    global_logger.error("[{}:{}]", __FILENAME__, __LINE__); \
    global_logger.error(__VA_ARGS__)

#else
#define SIMPLE_LOG_TRACE(...)
#define SIMPLE_LOG_DEBUG(...)
#define SIMPLE_LOG_INFO(...)
#define SIMPLE_LOG_WARN(...)
#define SIMPLE_LOG_ERROR(...)
#endif // CONFIG_SIMPLE_BASE_ENABLE_SPDLOG

#endif // SIMPLE_BASE_LOG_H_