#include "logger.hpp"

static const char* log_levels[] = {
    "debug",
    "info",
    "warning",
    "error",
    "critical"
};

Logger::Logger(void) {
    file = LittleFS.open("/log.txt", "a+");
    if (file) return;
    log(LL_CRITICAL, "Failed to open log file");
}

Logger::~Logger(void) {
    close();
}

// of course log_level can be out of bound but whatever
void Logger::log(log_level level, const char* message) {
    String entry = String(millis()) + " [" + log_levels[level] + "] " + message + "\n";
    file.write(entry.c_str());
    Serial.print(entry);
    if (level >= LL_ERROR) flush();
}

void Logger::log(log_level level, const String& message) {
    String entry = String(millis()) + " [" + log_levels[level] + "] " + message + "\n";
    file.write(entry.c_str());
    Serial.print(entry);
    if (level >= LL_ERROR) flush();
}

void Logger::flush(void) {
    file.flush();
    Serial.flush();
}

bool Logger::ok(void) {
    return !!file;
}

void Logger::close(void) {
    file.close();
}

void Logger::panic(const char* message) {
    log(LL_CRITICAL, message);
    close();
    ESP.deepSleep(0);
}

void Logger::panic(const String& message) {
    log(LL_CRITICAL, message);
    close();
    ESP.deepSleep(0);
}