#include "build_info.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <Ticker.h>
#include "logger.hpp"
#include "settings.hpp"
#include "utils.hpp"
#include "webUI.hpp"

Ticker ticker_blink;
Ticker ticker_flush;
Ticker ticker_debug;

WiFiEventHandler stationConnectedHandler;
WiFiEventHandler stationDisconnectedHandler;

// TODO: LOG IP
void onStationConnected(const WiFiEventSoftAPModeStationConnected& evt) {
    logger.log(LL_INFO, "Station connected: " + macToString(evt.mac));
}

void onStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& evt) {
    logger.log(LL_INFO, "Station disconnected: " + macToString(evt.mac));
}

void print_debug(void) {
    logger.log(LL_DEBUG, "DEBUG:");
    logger.log(LL_DEBUG, "  VCC: " + String(ESP.getVcc()));
    logger.log(LL_DEBUG, "  Chip ID: " + String(ESP.getChipId()));
    logger.log(LL_DEBUG, "  Full version: " + ESP.getFullVersion());
    logger.log(LL_DEBUG, "  Boot version: " + String(ESP.getBootVersion()));
    logger.log(LL_DEBUG, "  Boot mode: " + String(ESP.getBootMode()));
    logger.log(LL_DEBUG, "  CPU freq: " + String(ESP.getCpuFreqMHz()));
    logger.log(LL_DEBUG, "  Flash chip ID: " + String(ESP.getFlashChipId()));
    logger.log(LL_DEBUG, "  Flash chip vendor ID: " + String(ESP.getFlashChipVendorId()));
    logger.log(LL_DEBUG, "  Flash chip size: " + String(ESP.getFlashChipSize()));
    logger.log(LL_DEBUG, "  Flash chip real size: " + String(ESP.getFlashChipRealSize()));
    logger.log(LL_DEBUG, "  Flash chip speed: " + String(ESP.getFlashChipSpeed()));
    logger.log(LL_DEBUG, "  Sketch size: " + String(ESP.getSketchSize()) + " (" + String(ESP.getFreeSketchSpace()) + " free)");
    logger.log(LL_DEBUG, "  Sketch MD5: " + ESP.getSketchMD5());

    FSInfo fsinfo;
    if (!LittleFS.info(fsinfo)) {
        logger.log(LL_WARNING, "Failed to get filesystem stats");
        return;
    }
    logger.log(LL_DEBUG, "  FS total bytes: " + String(fsinfo.totalBytes));
    logger.log(LL_DEBUG, "  FS used bytes: " + String(fsinfo.usedBytes));
    logger.log(LL_DEBUG, "  FS block size: " + String(fsinfo.blockSize));
    logger.log(LL_DEBUG, "  FS page size: " + String(fsinfo.pageSize));
    logger.log(LL_DEBUG, "  FS max open files: " + String(fsinfo.maxOpenFiles));
    logger.log(LL_DEBUG, "  FS max path length: " + String(fsinfo.maxPathLength));
}

void setup() {
    Serial.begin(9600);
    Serial.println();

    if (!LittleFS.begin()) esp_exit("failed to initialize filesystem");

    _logger = new Logger();
    if (!logger.ok()) esp_exit("failed to start logger");
    logger.log(LL_INFO, "started logfile");

    logger.log(LL_INFO, "Starting: " VERSION);

    print_debug();
    settings_read_all();

    logger.log(LL_INFO, "Starting AP, with SSID: `" + wifi_ssid + "` and password `" + wifi_pass + "`");
    WiFi.persistent(false);
    WiFi.mode(WIFI_AP);
    IPAddress ap_ip(192, 168, 0, 1);
    WiFi.softAPConfig(ap_ip, ap_ip, IPAddress(255, 255, 255, 0));
    WiFi.softAP(wifi_ssid, wifi_pass);
    logger.log(LL_INFO, "AP started, IP address: " + WiFi.softAPIP().toString() + " (should be " + ap_ip.toString() + ")"); // TODO: print ssid and password

    stationConnectedHandler = WiFi.onSoftAPModeStationConnected(&onStationConnected);
    stationDisconnectedHandler = WiFi.onSoftAPModeStationDisconnected(&onStationDisconnected);

    init_web_ui();

    ticker_flush.attach_ms(30 * 1000, []() { logger.flush(); });
    ticker_debug.attach_ms(5 * 1000, []() {
        static uint8_t i = 0;
        i = (i + 1) % 24; // 120 seconds
        if (!i) logger.log(LL_DEBUG, "Heap fragmentation: " + String(ESP.getHeapFragmentation()) + "%");
        uint32_t free = ESP.getFreeHeap();
        if (free <= 4096) logger.log(LL_WARNING, "Low heap: " + String(free) + " bytes");
    });
    ticker_blink.attach_ms(500, []() { digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); });

    pinMode(LED_BUILTIN, OUTPUT);

    logger.log(LL_INFO, "Ready, have fun :3");
}

void loop() {
    server.handleClient();
}
