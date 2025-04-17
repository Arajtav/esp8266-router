#ifndef _LOGGER_HPP
#define _LOGGER_HPP

#include <Arduino.h>
#include <LittleFS.h>

// WHY
#undef panic

typedef enum {
    LL_DEBUG = 0,
    LL_INFO,
    LL_WARNING,
    LL_ERROR,
    LL_CRITICAL
} log_level;

class Logger {
    private:
        void close(void); // kinda unsafe
        File file;

    public:
        Logger(void);
        ~Logger(void);
        void log(log_level, const char*);
        void log(log_level, const String&);
        void flush(void);
        bool ok(void);
        void panic(const char*);
        void panic(const String&);
};

#endif