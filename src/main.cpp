#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include "logger.hpp"
#include "settings.hpp"
#include "utils.hpp"

WiFiEventHandler stationConnectedHandler;
WiFiEventHandler stationDisconnectedHandler;

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
    Serial.println();

    if (!LittleFS.begin()) esp_exit("failed to initialize filesystem");

    _logger = new Logger();
    if (!logger.ok()) esp_exit("failed to start logger");
    logger.log(LL_INFO, "started logfile");

    settings_read_all();

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
