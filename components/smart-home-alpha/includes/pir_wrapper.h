#ifndef PIR_WRAPPER_H
#define PIR_WRAPPER_H

struct pir_wrapper_t {
    void (*on_movement_start)();
    void (*on_movement_stop)();
    int status;
    int flag_;
    int data_pin_num;
};

void pir_initialize(struct pir_wrapper_t *pir_wrapper);
void pir_start(struct pir_wrapper_t *pir_wrapper);
void pir_stop(struct pir_wrapper_t *pir_wrapper);

#endif