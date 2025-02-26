#ifndef PCB_WRAPPER_H
#define PCB_WRAPPER_H

#include "dht_wrapper.h"
#include "ldr_wrapper.h"
#include "pir_wrapper.h"
#include "led_wrapper.h"

struct pcb_wrapper_t {
    struct dht_wrapper_t dht_wrapper;
    struct ldr_wrapper_t ldr_wrapper;
    struct led_wrapper_t led_wrapper;
    struct pir_wrapper_t pir_wrapper;
    int movement;
};

extern struct pcb_wrapper_t pcb_wrapper;

#endif