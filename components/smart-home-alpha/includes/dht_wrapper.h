#ifndef DHT_WRAPPER_H
#define DHT_WRAPPER_H

struct dht_wrapper_t {
    int data_pin_num;

    float temperature;
    float humidity;

    int delay_ms;

    int isRunning;
    int flag_;
};

void dht_wrapper_initialize(struct dht_wrapper_t *dht_wrapper);
void dht_wrapper_start(struct dht_wrapper_t *dht_wrapper);
void dht_wrapper_stop(struct dht_wrapper_t *dht_wrapper);

#endif