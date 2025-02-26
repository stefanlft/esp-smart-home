#ifndef PIEZO_WRAPPER_H
#define PIEZO_WRAPPER_H

#include "stdint.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

struct piezo_wrapper_t {
    int piezo_pin_num;
    
    float power;
    uint32_t frequency;

    int isRunning;
    int flag_;

    TaskHandle_t task_handle_;
    SemaphoreHandle_t mutex;
};

struct piezo_note_t {
    uint32_t frequency;
    uint32_t duration_ms;
};


void piezo_wrapper_initialize(struct piezo_wrapper_t *piezo_wrapper);
void piezo_wrapper_on(struct piezo_wrapper_t *piezo_wrapper);
void piezo_wrapper_off(struct piezo_wrapper_t *piezo_wrapper);
void piezo_wrapper_set_power(struct piezo_wrapper_t *piezo_wrapper, float power);
void piezo_wrapper_set_frequency(struct piezo_wrapper_t *piezo_wrapper, uint32_t frequency);
void piezo_wrapper_play_note(struct piezo_wrapper_t *piezo_wrapper, struct piezo_note_t *note);
void piezo_wrapper_play_song(struct piezo_wrapper_t *piezo_wrapper, struct piezo_note_t *song, uint32_t song_length);

#endif