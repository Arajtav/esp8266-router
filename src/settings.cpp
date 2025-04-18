#include <Arduino.h>
#include <LittleFS.h>
#include "settings.hpp"
#include "logger.hpp"

String admin_username;
String admin_password;
String wifi_ssid;
String wifi_pass;

void settings_read_all() {
    settings_read_admin();
    settings_read_wifi();
}

void settings_read_wifi(void) {
    File file = LittleFS.open("/config/wifi", "r");
    if (!file) logger.panic("missing `/config/wifi`");
    String line = file.readStringUntil('\n');
    file.close();
    int separatorIndex = line.indexOf(':');
    if (separatorIndex < 0) logger.panic("`config/wifi` is invalid");
    wifi_ssid = line.substring(0, separatorIndex);
    wifi_pass = line.substring(separatorIndex + 1);
}

void settings_read_admin(void) {
    File file = LittleFS.open("/config/admin", "r");
    if (!file) logger.panic("missing `/config/admin`");
    String line = file.readStringUntil('\n');
    file.close();
    int separatorIndex = line.indexOf(':');
    if (separatorIndex < 0) logger.panic("`config/admin` is invalid");
    admin_username = line.substring(0, separatorIndex);
    admin_password = line.substring(separatorIndex + 1);
}