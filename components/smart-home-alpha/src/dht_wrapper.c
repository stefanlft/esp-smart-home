#include "dht_wrapper.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "dht.h"

void dht_task(void *pvParameters) {
    struct dht_wrapper_t *dht_wrapper = *(struct dht_wrapper_t **)pvParameters;

    vTaskDelay(pdMS_TO_TICKS(1000));

#ifdef CONFIG_EXAMPLE_INTERNAL_PULLUP
    gpio_set_pull_mode(dht_gpio, GPIO_PULLUP_ONLY);
#endif

    while (1) {
        if (dht_read_float_data(DHT_TYPE_DHT11, dht_wrapper->data_pin_num, &dht_wrapper->humidity, &dht_wrapper->temperature) == ESP_OK)
            ESP_LOGD("DHT", "Humidity: %.1f%% Temp: %.1fC", dht_wrapper->humidity, dht_wrapper->temperature);
        else
            ESP_LOGW("DHT", "Could not read data from sensor");

        vTaskDelay(pdMS_TO_TICKS(dht_wrapper->delay_ms));
    }
}

void dht_wrapper_initialize(struct dht_wrapper_t *dht_wrapper) {
    dht_wrapper->data_pin_num = 0;
    dht_wrapper->flag_ = 0;
    dht_wrapper->isRunning = 0;
    dht_wrapper->delay_ms = 2000;
}

void dht_wrapper_start(struct dht_wrapper_t *dht_wrapper) {
    dht_wrapper->isRunning = 1;

    if (dht_wrapper->flag_ == 0) {
        dht_wrapper->flag_ = 1;

        xTaskCreate(dht_task, "dht_task", configMINIMAL_STACK_SIZE * 4, &dht_wrapper, 5, NULL);
    }
}

void dht_wrapper_stop(struct dht_wrapper_t *dht_wrapper) {
    dht_wrapper->isRunning = 0;
}