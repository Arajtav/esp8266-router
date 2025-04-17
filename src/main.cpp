#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>

#define LOG_INFO "info"
#define LOG_DEBUG "debug"
#define LOG_WARNING "warning"
#define LOG_ERROR "error"
#define LOG_CRITICAL "critical"

static File log_file;

WiFiEventHandler stationConnectedHandler;
WiFiEventHandler stationDisconnectedHandler;

String macToString(const unsigned char* mac) {
    char buf[18]; // 6*2 (numbers) + 5 (colons) + 1 (null terminator)
    snprintf(buf, 18, "%02x:%02x:%02x:%02x:%02x:%02x", MAC2STR(mac));
    return String(buf);
}

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
    log_file.flush();
    delay(1000);
    ESP.deepSleep(0);
}

static String admin_username;
static String admin_password;
static String wifi_ssid;
static String wifi_pass;

void read_wifi() {
    File file = LittleFS.open("/config/wifi", "r");
    if (!file) cpanic("missing `/config/wifi`");
    String line = file.readStringUntil('\n');
    file.close();
    int separatorIndex = line.indexOf(':');
    if (separatorIndex < 0) cpanic("`config/wifi` is invalid");
    wifi_ssid = line.substring(0, separatorIndex);
    wifi_pass = line.substring(separatorIndex + 1);
}

void read_admin() {
    File file = LittleFS.open("/config/admin", "r");
    if (!file) cpanic("missing `/config/admin`");
    String line = file.readStringUntil('\n');
    file.close();
    int separatorIndex = line.indexOf(':');
    if (separatorIndex < 0) cpanic("`config/admin` is invalid");
    admin_username = line.substring(0, separatorIndex);
    admin_password = line.substring(separatorIndex + 1);
}

// TODO: LOG IP
void onStationConnected(const WiFiEventSoftAPModeStationConnected& evt) {
    clog(LOG_INFO, "Station connected: " + macToString(evt.mac));
}

void onStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& evt) {
    clog(LOG_INFO, "Station disconnected: " + macToString(evt.mac));
}

IPAddress ap_ip(192, 168, 0, 1);

void setup() {
    Serial.begin(9600);

    if (!LittleFS.begin()) cpanic("failed to initialize filesystem");

    log_file = LittleFS.open("/log.txt", "a+");
    if (!log_file) cpanic("Failed to open log file");

    clog(LOG_INFO, "Started logfile");
    read_admin();
    read_wifi();
    clog(LOG_INFO, "Starting AP, with SSID: `" + wifi_ssid + "` and password `" + wifi_pass + "`");

    WiFi.persistent(false);
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(ap_ip, ap_ip, IPAddress(255, 255, 255, 0));
    WiFi.softAP(wifi_ssid, wifi_pass);
    clog(LOG_INFO, "AP started, IP address: " + WiFi.softAPIP().toString() + " (should be " + ap_ip.toString() + ")"); // TODO: print ssid and password

    stationConnectedHandler = WiFi.onSoftAPModeStationConnected(&onStationConnected);
    stationDisconnectedHandler = WiFi.onSoftAPModeStationDisconnected(&onStationDisconnected);
}

void loop() {
    log_file.flush();
    delay(5000);
}
