#include "led_wrapper.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "driver/ledc.h"

void led_task(void *pvParameters) {
    struct led_wrapper_t *led_wrapper = *(struct led_wrapper_t **)pvParameters;

    if (led_wrapper == NULL) {
        ESP_LOGE("LED", "SHIT IS VERY FUCKING WRONG!");
        vTaskDelete(NULL);
        return;
    }

    vTaskDelay(pdMS_TO_TICKS(100));

    ledc_channel_config_t pwm_config = {
        .channel = LEDC_CHANNEL_0,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .gpio_num = led_wrapper->led_pin_num,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0
    };

    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 5000
    };

    gpio_set_direction(led_wrapper->led_pin_num, GPIO_MODE_OUTPUT);

    ledc_timer_config(&ledc_timer);
    ledc_channel_config(&pwm_config);

    ESP_LOGD("LED", "Task running...");

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));


        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, led_wrapper->isRunning*led_wrapper->intensity*1023);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
    }
}


void led_wrapper_initialize(struct led_wrapper_t *led_wrapper) {
    led_wrapper->flag_ = 0;
    led_wrapper->isRunning = 0;
    led_wrapper->led_pin_num = 0;
    led_wrapper->intensity = 1;
}

void led_wrapper_on(struct led_wrapper_t *led_wrapper) {
    led_wrapper->isRunning = 1;

    if (led_wrapper->flag_ == 0) {
        led_wrapper->flag_ = 1;

        xTaskCreate(led_task, "led_task", configMINIMAL_STACK_SIZE * 4, &led_wrapper, 5, NULL);
    }
}

void led_wrapper_off(struct led_wrapper_t *led_wrapper) {
    led_wrapper->isRunning = 0;
}

void led_wrapper_set_intensity(struct led_wrapper_t *led_wrapper, float intensity) {
    if (intensity != (int)intensity || intensity == 0) 
        led_wrapper->intensity = intensity - (int) intensity;
    else
        led_wrapper->intensity = 1;
}