#ifndef LDR_WRAPPER_H
#define LDR_WRAPPER_H

struct ldr_wrapper_t {
    int read_pin_num;
    
    int raw_value;
    float percent;

    int delay_ms;

    int isRunning;
    int flag_;
};

void ldr_wrapper_initialize(struct ldr_wrapper_t *ldr_wrapper);
void ldr_wrapper_start(struct ldr_wrapper_t *ldr_wrapper);
void ldr_wrapper_stop(struct ldr_wrapper_t *ldr_wrapper);

#endif