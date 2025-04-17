#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include "logger.hpp"

WiFiEventHandler stationConnectedHandler;
WiFiEventHandler stationDisconnectedHandler;

String macToString(const unsigned char* mac) {
    char buf[18]; // 6*2 (numbers) + 5 (colons) + 1 (null terminator)
    snprintf(buf, 18, "%02x:%02x:%02x:%02x:%02x:%02x", MAC2STR(mac));
    return String(buf);
}

static Logger logger;

static String admin_username;
static String admin_password;
static String wifi_ssid;
static String wifi_pass;

void read_wifi() {
    File file = LittleFS.open("/config/wifi", "r");
    if (!file) logger.panic("missing `/config/wifi`");
    String line = file.readStringUntil('\n');
    file.close();
    int separatorIndex = line.indexOf(':');
    if (separatorIndex < 0) logger.panic("`config/wifi` is invalid");
    wifi_ssid = line.substring(0, separatorIndex);
    wifi_pass = line.substring(separatorIndex + 1);
}

void read_admin() {
    File file = LittleFS.open("/config/admin", "r");
    if (!file) logger.panic("missing `/config/admin`");
    String line = file.readStringUntil('\n');
    file.close();
    int separatorIndex = line.indexOf(':');
    if (separatorIndex < 0) logger.panic("`config/admin` is invalid");
    admin_username = line.substring(0, separatorIndex);
    admin_password = line.substring(separatorIndex + 1);
}

// TODO: LOG IP
void onStationConnected(const WiFiEventSoftAPModeStationConnected& evt) {
    logger.log(LL_INFO, "Station connected: " + macToString(evt.mac));
}

void onStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& evt) {
    logger.log(LL_INFO, "Station disconnected: " + macToString(evt.mac));
}

IPAddress ap_ip(192, 168, 0, 1);

void setup() {
    Serial.begin(9600);

    if (!LittleFS.begin()) {
        Serial.println("failed to initialize filesystem");
        Serial.flush();
        ESP.deepSleep(0);
    }

    logger = Logger();
    if (!logger.ok()) ESP.deepSleep(0);

    logger.log(LL_INFO, "Started logfile");
    read_admin();
    read_wifi();
    logger.log(LL_INFO, "Starting AP, with SSID: `" + wifi_ssid + "` and password `" + wifi_pass + "`");

    WiFi.persistent(false);
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(ap_ip, ap_ip, IPAddress(255, 255, 255, 0));
    WiFi.softAP(wifi_ssid, wifi_pass);
    logger.log(LL_INFO, "AP started, IP address: " + WiFi.softAPIP().toString() + " (should be " + ap_ip.toString() + ")"); // TODO: print ssid and password

    stationConnectedHandler = WiFi.onSoftAPModeStationConnected(&onStationConnected);
    stationDisconnectedHandler = WiFi.onSoftAPModeStationDisconnected(&onStationDisconnected);
}

void loop() {
    logger.flush();
    delay(5000);
}
