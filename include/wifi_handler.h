#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H

#include "esp_wifi.h"

void initialize_wifi();
uint16_t scan_for_aps(wifi_ap_record_t **ap_info);

void start_periodic_ap_scan();

struct wifi_scan {
    wifi_ap_record_t *ap_info;
    uint16_t ap_count;
};

extern struct wifi_scan periodic_wifi_scan;

#endif
