#ifndef SIMPLE_BASE_LOG_H_
#define SIMPLE_BASE_LOG_H_

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#ifdef __ANDROID__
#include <android/log.h>
#endif

class Loger {
public:
    typedef enum {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
    } LogLevel;

    static Loger& Instance() {
        static Loger loger;
        return loger;
    }

    Loger() : log_level_(DEBUG), log_tag_("") {}
    void set_log_level(LogLevel log_level) { log_level_ = log_level; }
    void set_log_tag(const char* tag) {
        if (tag != NULL)
            log_tag_ = tag;
    }
    void
    log(LogLevel level, const char* file, const char* function, int line, const char* fmt, ...) {
        const int kLogHanderSize = 1024;
        // if (level < log_level_)
        //     return;
        char log_header[kLogHanderSize];
#ifdef __ANDROID__
        snprintf(log_header,
                 kLogHanderSize,
                 "[%s]:[%s(%d)] %s",
                 function,
                 FindFileName(file),
                 line,
                 fmt);
#else
        struct timeval tv;
        struct tm* ptm;
        size_t offset                      = 0;
        static const char* log_level_str[] = {"D", "I", "W", "E"};

        gettimeofday(&tv, NULL);
        ptm    = localtime(&tv.tv_sec);
        offset = strftime(log_header, kLogHanderSize, "%m-%d %H:%M:%S", ptm);
        snprintf(log_header + offset,
                 kLogHanderSize,
                 ".%03ld %s %s [%s]:[%s(%d)] %s",
                 tv.tv_usec / 1000,
                 log_tag_,
                 log_level_str[level],
                 function,
                 FindFileName(file),
                 line,
                 fmt);
#endif

        va_list ap;
        va_start(ap, fmt);
#ifdef __ANDROID__
        __android_log_vprint(ANDROID_LOG_DEBUG + level, log_tag_, log_header, ap);
#else
        vprintf(log_header, ap);
#endif
        va_end(ap);
    }

private:
    LogLevel log_level_;
    const char* log_tag_;

    const char* FindFileName(const char* file) {
        int i = strlen(file);
        while (i >= 0 && file[i] != '/')
            --i;
        return file + i + 1;
    }
};

inline void set_level(Loger::LogLevel level) {
    Loger::Instance().set_log_level(level);
}

inline void set_tag(const char* tag) {
    Loger::Instance().set_log_tag(tag);
}


// #ifdef CONFIG_SIMPLE_BASE_ENABLE_SPDLOG
#define SIMPLE_LOG_DEBUG(fmt, ...) \
    Loger::Instance().log(Loger::DEBUG, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define SIMPLE_LOG_INFO(fmt, ...) \
    Loger::Instance().log(Loger::INFO, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define SIMPLE_LOG_WARN(fmt, ...) \
    Loger::Instance().log(Loger::WARNING, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define SIMPLE_LOG_ERROR(fmt, ...) \
    Loger::Instance().log(Loger::ERROR, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
// #else
// #define SIMPLE_LOG_DEBUG(fmt, ...)
// #define SIMPLE_LOG_INFO(fmt, ...)
// #define SIMPLE_LOG_WARN(fmt, ...)
// #define SIMPLE_LOG_ERROR(fmt, ...)
// #endif

#endif // SIMPLE_BASE_LOG_H_