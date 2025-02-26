#ifndef LED_WRAPPER_H
#define LED_WRAPPER_H

struct led_wrapper_t {
    int led_pin_num;
    
    float intensity;

    int isRunning;
    int flag_;
};

void led_wrapper_initialize(struct led_wrapper_t *led_wrapper);
void led_wrapper_on(struct led_wrapper_t *led_wrapper);
void led_wrapper_off(struct led_wrapper_t *led_wrapper);
void led_wrapper_set_intensity(struct led_wrapper_t *led_wrapper, float intensity);

#endif