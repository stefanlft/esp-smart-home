#include "piezo_wrapper.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "driver/ledc.h"

void piezo_task(void *pvParameters) {
    struct piezo_wrapper_t *piezo_wrapper = (struct piezo_wrapper_t *)pvParameters;

    if (piezo_wrapper == NULL) {
        ESP_LOGE("PIEZO", "SHIT IS VERY FUCKING WRONG!");
        vTaskDelete(NULL);
        return;
    }

    vTaskDelay(pdMS_TO_TICKS(100));

    ledc_channel_config_t pwm_config = {
        .channel = LEDC_CHANNEL_1,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .gpio_num = piezo_wrapper->piezo_pin_num,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_1,
        .duty = 0
    };

    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_1,
        .freq_hz = 5000
    };

    gpio_set_direction(piezo_wrapper->piezo_pin_num, GPIO_MODE_OUTPUT);

    ledc_timer_config(&ledc_timer);
    ledc_channel_config(&pwm_config);

    ESP_LOGD("PIEZO", "Task running...");

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));


        uint32_t duty = piezo_wrapper->isRunning ? (uint32_t)(piezo_wrapper->power * 1023) : 0;

        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, duty);
        ledc_set_freq(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_1, piezo_wrapper->frequency);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1);
    }
}


void piezo_wrapper_initialize(struct piezo_wrapper_t *piezo_wrapper) {
    piezo_wrapper->piezo_pin_num = -1;

    piezo_wrapper->isRunning = 0;

    piezo_wrapper->frequency = 100;
    piezo_wrapper->power = 0.5;

    piezo_wrapper->task_handle_ = NULL;
    piezo_wrapper->mutex = xSemaphoreCreateMutex(); // Create the mutex
    if (piezo_wrapper->mutex == NULL) {
        ESP_LOGE("PIEZO", "Failed to create mutex!");
    }

}

void piezo_wrapper_on(struct piezo_wrapper_t *piezo_wrapper) {
    piezo_wrapper->isRunning = 1;
     
    if (piezo_wrapper->task_handle_ == NULL) { // Check if task is already running
        BaseType_t xReturned = xTaskCreate(piezo_task, "piezo_task", configMINIMAL_STACK_SIZE * 2, piezo_wrapper, 5, &piezo_wrapper->task_handle_);
        if (xReturned != pdPASS) {
            ESP_LOGE("PIEZO", "Failed to create piezo task");
        }
    }
}

void piezo_wrapper_off(struct piezo_wrapper_t *piezo_wrapper) {
    piezo_wrapper->isRunning = 0;
}

void piezo_wrapper_set_power(struct piezo_wrapper_t *piezo_wrapper, float power) {
    if (power > 1.0f) power = 1.0f;
    if (power < 0.0f) power = 0.0f;
    piezo_wrapper->power = power;
}


void piezo_wrapper_set_frequency(struct piezo_wrapper_t *piezo_wrapper, uint32_t frequency) {
    piezo_wrapper->frequency = frequency;
}

void piezo_wrapper_play_note(struct piezo_wrapper_t *piezo_wrapper, struct piezo_note_t *note) {
    piezo_wrapper_on(piezo_wrapper);

    piezo_wrapper->isRunning = 1;

    uint32_t original = piezo_wrapper->frequency;


    if (note->frequency == 0) {
        piezo_wrapper_off(piezo_wrapper);
    } else {
        piezo_wrapper_set_frequency(piezo_wrapper, note->frequency);
    }
    
    vTaskDelay(pdMS_TO_TICKS(note->duration_ms));

    piezo_wrapper_off(piezo_wrapper);
    piezo_wrapper_set_frequency(piezo_wrapper, original);
}

struct wrapper_song_t {
    struct piezo_wrapper_t *wrapper;
    struct piezo_note_t *song;
    uint32_t song_length;
};

void piezo_play_song_task(void *pvParameters) {
    struct wrapper_song_t *wrapper_song = *(struct wrapper_song_t **)pvParameters;

    if (wrapper_song->wrapper == NULL || wrapper_song->song == NULL) {
        ESP_LOGE("PIEZO", "Invalid wrapper or song pointer!");
        ESP_LOGE("PIEZO", "Wrapper: %d Song: %d", wrapper_song->wrapper == NULL, wrapper_song->song == NULL);
        vTaskDelete(NULL);
        return;
    }    

    uint32_t original = wrapper_song->wrapper->frequency;


    for (uint32_t i = 0; i < wrapper_song->song_length; ++i) {
        ESP_LOGI("PIEZO", "Playing %dHz for %dms", (int)wrapper_song->song[i].frequency, (int)wrapper_song->song[i].duration_ms);
        piezo_wrapper_play_note(wrapper_song->wrapper, &wrapper_song->song[i]);
    }

    wrapper_song->wrapper->frequency = original;

    vTaskDelete(NULL);
}

void piezo_wrapper_play_song(struct piezo_wrapper_t *piezo_wrapper, struct piezo_note_t *song, uint32_t song_length) {

    

    if (xSemaphoreTake(piezo_wrapper->mutex, portMAX_DELAY) == pdTRUE) { // Protect access

        if (song == NULL) {
            ESP_LOGE("PIEZO", "Invalid song pointer!");
            return;
        }

        xTaskCreate(piezo_play_song_task, "piezo_song_task", configMINIMAL_STACK_SIZE * 4, &(struct wrapper_song_t){
            piezo_wrapper, song, song_length
        }, 5, NULL);


        xSemaphoreGive(piezo_wrapper->mutex); // Release the mutex
    } else {
        ESP_LOGE("PIEZO", "Failed to take mutex!");
    }


    
}