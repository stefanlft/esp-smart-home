#ifndef UTILS_H
#define UTILS_H

#include "esp_wifi.h"

void dump_mac_address(const uint8_t mac[6], const char *mac_str);
const char* get_auth_mode_name(wifi_auth_mode_t authmode);

#endif
