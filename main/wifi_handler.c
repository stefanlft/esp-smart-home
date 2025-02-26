#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_mac.h"

#include "esp_event.h"
#include "nvs_flash.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include <portmacro.h>
#include <string.h>

#include "utils.h"
#include "wifi_handler.h"

static const char *TAG_WIFI = "WIFI_HANDLER";

struct wifi_scan periodic_wifi_scan;

void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base != WIFI_EVENT) {
        return;
    }

    switch (event_id) {
        case (WIFI_EVENT_AP_STACONNECTED): {
            wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;

            char mac_str[18] = {0};
            dump_mac_address(event->mac, mac_str);
            ESP_LOGI(TAG_WIFI, "Client connected! MAC: %s", mac_str);
            
            break;
        }

        case (WIFI_EVENT_AP_STADISCONNECTED): {
            wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
            
            char mac_str[18] = {0};
            dump_mac_address(event->mac, mac_str);
            ESP_LOGI(TAG_WIFI, "Client disconnected! MAC: %s", mac_str);

            break;
        }
        
        default:
            break;
    }
}

void initialize_wifi() {
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_ap();  // Use AP mode network interface
    esp_netif_create_default_wifi_sta(); // Use STA mode to scan

    wifi_init_config_t init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&init_config));

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL);

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = "ESP32_MARLENE",
            .ssid_len = strlen("ESP32_MARLENE"),
            .password = "MARLENE1",
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA2_PSK
        }
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));  // Set mode to Access Point AND Station
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));  // Apply config to AP
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG_WIFI, "Wi-Fi AP Mode Initialized");
}

uint16_t scan_for_aps(wifi_ap_record_t **ap_info) {
    uint16_t ap_count = 0;

    ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));

    *ap_info = malloc(sizeof(wifi_ap_record_t) * ap_count);

    if (ap_count == 0) {
        ESP_LOGW(TAG_WIFI, "No access points found.");
        *ap_info = NULL; // Ensure the pointer is NULL in this case
        return 0;
    }

    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, *ap_info));
    if (*ap_info == NULL) {
        ESP_LOGE(TAG_WIFI, "Failed to allocate memory for AP records.");
        return 0;
    }

    ESP_LOGI("WIFI_SCAN", "Found %d access points:", ap_count);

    return ap_count;
}

void scan_task(void *pvParameters) {
    while (1) {
        periodic_wifi_scan.ap_count = scan_for_aps(&periodic_wifi_scan.ap_info);

        ESP_LOGI(TAG_WIFI, "Found %d APs", periodic_wifi_scan.ap_count);

        if (periodic_wifi_scan.ap_info != NULL) {  // Ensure we only process valid data
            for (int i = 0; i < periodic_wifi_scan.ap_count; i++) {
                char mac_str[18] = {0};
                dump_mac_address(periodic_wifi_scan.ap_info[i].bssid, mac_str);

                ESP_LOGI("WIFI_SCAN", "SSID: %s | BSSID: %s | RSSI: %d | CH: %d | Authmode: %s",
                            periodic_wifi_scan.ap_info[i].ssid, mac_str,
                            periodic_wifi_scan.ap_info[i].rssi, periodic_wifi_scan.ap_info[i].primary, get_auth_mode_name(periodic_wifi_scan.ap_info[i].authmode));
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10000)); // Scan every 10 seconds

        if (periodic_wifi_scan.ap_info != NULL) {
            free(periodic_wifi_scan.ap_info);
        }

        ESP_LOGI(TAG_WIFI, "%d APs and %d size", periodic_wifi_scan.ap_count, sizeof(periodic_wifi_scan.ap_info));
    }
}

void start_periodic_ap_scan() {
    xTaskCreate(scan_task, "WiFi_Scan_Task", 4096, NULL, 5, NULL);
}