idf_component_register(
    SRC_DIRS "src"
    SRCS "src/dht_wrapper.c" "src/pir_wrapper.c" "src/piezo_wrapper.c" "src/led_wrapper.c" "src/ldr_wrapper.c"
    INCLUDE_DIRS "includes"
    REQUIRES driver dht esp_adc
)