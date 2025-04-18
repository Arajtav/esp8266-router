#ifndef _SETTINGS_HPP
#define _SETTINGS_HPP

#include <Arduino.h>

extern String admin_username;
extern String admin_password;
extern String wifi_ssid;
extern String wifi_pass;

void settings_read_all(void); // reloads everything from config files
void settings_read_admin(void); // reloads admin username and password hash from the config file
void settings_read_wifi(void); // reloads wifi ssid and password from the confing file

#endif