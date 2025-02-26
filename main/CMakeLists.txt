idf_component_register(
    SRCS "web_handler.c" "utils.c" "wifi_handler.c" "main.c"
    INCLUDE_DIRS "."
    REQUIRES smart-home-alpha esp_wifi nvs_flash esp_http_server json
)