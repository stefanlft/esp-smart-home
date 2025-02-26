#include "ldr_wrapper.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"

void ldr_task(void *pvParameters) {
    struct ldr_wrapper_t *ldr_wrapper = *(struct ldr_wrapper_t **)pvParameters;

    vTaskDelay(pdMS_TO_TICKS(100));

    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_new_unit(&init_cfg, &adc1_handle);
    
    adc_oneshot_chan_cfg_t channel_cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_6, &channel_cfg);

    while (1) {
        adc_oneshot_read(adc1_handle, ADC_CHANNEL_6, &ldr_wrapper->raw_value);
        ldr_wrapper->percent = ldr_wrapper->raw_value * 100 / 4096;

        ESP_LOGD("LDR", "Value: %d", ldr_wrapper->raw_value);
        ESP_LOGD("LDR", "Lightness: %.1f\n", ldr_wrapper->percent);
        
        vTaskDelay(pdMS_TO_TICKS(ldr_wrapper->delay_ms));  // 500ms delay
    }
}

void ldr_wrapper_initialize(struct ldr_wrapper_t *ldr_wrapper) {
    ldr_wrapper->read_pin_num = 0;
    ldr_wrapper->flag_ = 0;
    ldr_wrapper->isRunning = 0;
    ldr_wrapper->delay_ms = 500;
}

void ldr_wrapper_start(struct ldr_wrapper_t *ldr_wrapper) {
    ldr_wrapper->isRunning = 1;

    if (ldr_wrapper->flag_ == 0) {
        ldr_wrapper->flag_ = 1;

        xTaskCreate(ldr_task, "ldr_task", configMINIMAL_STACK_SIZE * 4, &ldr_wrapper, 5, NULL);
    }
}

void ldr_wrapper_stop(struct ldr_wrapper_t *ldr_wrapper) {
    ldr_wrapper->isRunning = 0;
}