#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>

#define LOG_INFO "info"
#define LOG_DEBUG "debug"
#define LOG_WARNING "warning"
#define LOG_ERROR "error"
#define LOG_CRITICAL "critical"

static File log_file;

void clog(const char* level, const char* message) {
    String entry = String(millis()) + " [" + level + "] " + message + "\n";
    if (log_file) log_file.write(entry.c_str());
    Serial.print(entry);
}

void clog(const char* level, const String& message) {
    String entry = String(millis()) + " [" + level + "] " + message + "\n";
    if (log_file) log_file.write(entry.c_str());
    Serial.print(entry);
}

void cpanic(const char* message) {
    clog(LOG_CRITICAL, message);
    delay(1000);
    ESP.deepSleep(0);
}

void setup() {
    Serial.begin(9600);

    if (!LittleFS.begin()) cpanic("failed to initialize filesystem");

    log_file = LittleFS.open("/log.txt", "a+");
    if (!log_file) cpanic("Failed to open log file");

    File admin_file = LittleFS.open("/config/admin", "r");
    if (!admin_file) cpanic("missing `/config/admin`");

    clog(LOG_INFO, "admin entry:");
    while (admin_file.available()) clog(LOG_INFO, admin_file.readStringUntil('\n'));
    admin_file.close();
}

void loop() {
    log_file.flush();
    delay(5000);
}
