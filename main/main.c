#include <stdio.h>

#include "pcb_wrapper.h"

// #include "music_library.h"
#include "wifi_handler.h"
#include "web_handler.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>

#define DHT_GPIO CONFIG_DHT_GPIO
#define PIR_GPIO CONFIG_PIR_GPIO
#define PIEZO_GPIO CONFIG_PIEZO_GPIO
#define LED_GPIO CONFIG_LED_GPIO
#define LDR_GPIO CONFIG_LDR_GPIO

static const char *TAG = "main";
struct pcb_wrapper_t pcb_wrapper;

void on_movement_start() {
    ESP_LOGI(TAG, "Movement started!");

    notify_movement_start();
    led_wrapper_on(&pcb_wrapper.led_wrapper);
    vTaskDelay(pdMS_TO_TICKS(100));
    led_wrapper_off(&pcb_wrapper.led_wrapper);

    pcb_wrapper.movement = 1;
}

void on_movement_stop() {
    pcb_wrapper.movement = 0;
}

void app_main(void) {
    dht_wrapper_initialize(&pcb_wrapper.dht_wrapper);
    pcb_wrapper.dht_wrapper.data_pin_num = DHT_GPIO;
    dht_wrapper_start(&pcb_wrapper.dht_wrapper);

    ldr_wrapper_initialize(&pcb_wrapper.ldr_wrapper);
    pcb_wrapper.ldr_wrapper.read_pin_num = LDR_GPIO;
    ldr_wrapper_start(&pcb_wrapper.ldr_wrapper);

    pir_initialize(&pcb_wrapper.pir_wrapper);
    pcb_wrapper.pir_wrapper.data_pin_num = PIR_GPIO;
    pcb_wrapper.pir_wrapper.on_movement_start = on_movement_start;
    pcb_wrapper.pir_wrapper.on_movement_stop = on_movement_stop;
    pir_start(&pcb_wrapper.pir_wrapper);

    led_wrapper_initialize(&pcb_wrapper.led_wrapper);
    pcb_wrapper.led_wrapper.led_pin_num = LED_GPIO;
    led_wrapper_set_intensity(&pcb_wrapper.led_wrapper, 1);
    led_wrapper_off(&pcb_wrapper.led_wrapper);

    initialize_wifi();
    initialize_http();

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1800));

        ESP_LOGI(TAG, "Temperature: %.1f°C", pcb_wrapper.dht_wrapper.temperature);
        ESP_LOGI(TAG, "Humidity:    %.1f%%", pcb_wrapper.dht_wrapper.humidity);
        ESP_LOGI(TAG, "Lightness:   %.1f%%", pcb_wrapper.ldr_wrapper.percent);

        ESP_LOGI(TAG, "------------------------------------------");

        led_wrapper_on(&pcb_wrapper.led_wrapper);
        vTaskDelay(pdMS_TO_TICKS(200));
        led_wrapper_off(&pcb_wrapper.led_wrapper);
    }
    
}
