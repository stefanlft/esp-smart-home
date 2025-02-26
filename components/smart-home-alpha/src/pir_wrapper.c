#include "pir_wrapper.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "driver/gpio.h"

void pir_task(void *pvParameters) {
    struct pir_wrapper_t *pir_wrapper = *(struct pir_wrapper_t **)pvParameters;

    int previousState, currentState;

    previousState = 0;
    currentState = 0;

    ESP_LOGD("PIR", "pin: %d, status: %d", pir_wrapper->data_pin_num, pir_wrapper->status);

    gpio_set_direction(pir_wrapper->data_pin_num, GPIO_MODE_INPUT);

    // gpio_pulldown_en(GPIO_NUM_16);
    // gpio_pullup_dis(GPIO_NUM_16);

    while (1) {
        if (pir_wrapper->status == 0) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        vTaskDelay(pdMS_TO_TICKS(3000));

    
        currentState = gpio_get_level(pir_wrapper->data_pin_num);

        ESP_LOGD("PIR", "Current state: %d", currentState);

        if (currentState == previousState) {
            continue;
        }

        if (previousState == 0) {
            ESP_LOGI("PIR", "Motion detected!");
            if (pir_wrapper->on_movement_start != NULL)
                pir_wrapper->on_movement_start();
                
        } else {
            ESP_LOGI("PIR", "Motion stopped!");
            if (pir_wrapper->on_movement_stop != NULL)
                pir_wrapper->on_movement_stop();

        }

        previousState = currentState;

    }
}

void pir_initialize(struct pir_wrapper_t *pir_wrapper) {
    pir_wrapper->data_pin_num = 0;
    pir_wrapper->flag_ = 0;
    pir_wrapper->on_movement_start = NULL;
    pir_wrapper->on_movement_stop = NULL;
    pir_wrapper->status = 0;
}

void pir_start(struct pir_wrapper_t *pir_wrapper) {
    pir_wrapper->status = 1;

    if (pir_wrapper->flag_ == 0) {
        pir_wrapper->flag_ = 1;

        xTaskCreate(pir_task, "pir_task", configMINIMAL_STACK_SIZE * 4, &pir_wrapper, 5, NULL);
    }
}

void pir_stop(struct pir_wrapper_t *pir_wrapper) {
    pir_wrapper->status = 0;
}