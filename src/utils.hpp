#ifndef _UTILS_HPP
#define _UTILS_HPP

#include <Arduino.h>

void esp_exit(const char*);
void esp_exit(const String&);

String macToString(const unsigned char*);

String readFile(const char*);

String randomHex();

#endif