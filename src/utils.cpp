#include <Arduino.h>
#include <LittleFS.h>
#include "logger.hpp"
#include "utils.hpp"

void esp_exit(const char* message) {
    Serial.println(message);
    Serial.flush();
    LittleFS.end();
    ESP.deepSleep(0);
    Serial.println("Something went really wrong");
}

void esp_exit(const String& message) {
    Serial.println(message);
    Serial.flush();
    LittleFS.end();
    ESP.deepSleep(0);
    Serial.println("Something went really wrong");
}

String macToString(const unsigned char* mac) {
    char buf[18]; // 6*2 (numbers) + 5 (colons) + 1 (null terminator)
    snprintf(buf, 18, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(buf);
}

String readFile(const char* filename) {
    File file = LittleFS.open(filename, "r");
    if (!file) {
        logger.log(LL_ERROR, String("Failed to open file `") + filename + "`");
        return String("");
    }
    String data = file.readString();
    file.close();
    return data;
}
