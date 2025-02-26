#include "web_handler.h"
#include <esp_http_server.h>
#include "esp_log.h"
#include "esp_wifi.h"
#include "wifi_handler.h"
#include "utils.h"
#include "cJSON.h"
#include "pcb_wrapper.h"
#include "cJSON.h"


static const char *TAG_WEB = "WEB_HANDLER";

static httpd_req_t *movement_req = NULL;

esp_err_t get_ldr_handler(httpd_req_t *req) {
    ESP_LOGI(TAG_WEB, "Requested %s!", req->uri);
    char message[64];
    sprintf(message, "%.1f%%", pcb_wrapper.ldr_wrapper.percent);
    return httpd_resp_send(req, message, HTTPD_RESP_USE_STRLEN);;
}

esp_err_t get_dht_temp_handler(httpd_req_t *req) {
    ESP_LOGI(TAG_WEB, "Requested %s!", req->uri);
    char message[64];
    sprintf(message, "%.1f°C", pcb_wrapper.dht_wrapper.temperature);
    return httpd_resp_send(req, message, HTTPD_RESP_USE_STRLEN);;
}

esp_err_t get_dht_humid_handler(httpd_req_t *req) {
    ESP_LOGI(TAG_WEB, "Requested %s!", req->uri);
    char message[64];
    sprintf(message, "%.1f%%", pcb_wrapper.dht_wrapper.humidity);
    return httpd_resp_send(req, message, HTTPD_RESP_USE_STRLEN);;
}

esp_err_t get_json_handler(httpd_req_t *req) {
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        ESP_LOGE(TAG_WEB, "Failed to create JSON object");
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGD(TAG_WEB, "Created JSON array!");

    cJSON_AddNumberToObject(root, "lightness", pcb_wrapper.ldr_wrapper.percent);
    cJSON_AddNumberToObject(root, "humidity", pcb_wrapper.dht_wrapper.humidity);
    cJSON_AddNumberToObject(root, "temperature", pcb_wrapper.dht_wrapper.temperature);
    cJSON_AddNumberToObject(root, "motion", pcb_wrapper.movement);


    char *json_string = cJSON_Print(root);
    if (json_string == NULL) {
        ESP_LOGE(TAG_WEB, "Failed to print JSON object");
        cJSON_Delete(root);
        return ESP_ERR_NO_MEM;
    }

    esp_err_t res = httpd_resp_send(req, json_string, HTTPD_RESP_USE_STRLEN);
    if (res != ESP_OK) {
        ESP_LOGE(TAG_WEB, "Failed to send response");
    }

    free(json_string);  // Free the string returned by cJSON_Print
    cJSON_Delete(root); // Free the JSON object

    return res;
}

esp_err_t sse_handler(httpd_req_t *req) {
    movement_req = req;

    // Set response type for SSE
    httpd_resp_set_type(req, "text/event-stream");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    httpd_resp_set_hdr(req, "Connection", "keep-alive");
    httpd_resp_set_hdr(req, "Content-Type", "text/event-stream");

    ESP_LOGI(TAG_WEB, "Requested %s!", req->uri);

    // Send the SSE data
    while (1) {
        char message[64];
        sprintf(message, "event: keep-alive\ndata: null\n\n");
        httpd_resp_send(req, message, strlen(message));
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait for 1 second before sending next event
    }

    return ESP_OK;
}

int notify_movement_start() {
    if (movement_req == NULL) 
        return 1;

    char message[64];
    sprintf(message, "event: pir\ndata: movement started\n\n");
    httpd_resp_send(movement_req, message, strlen(message));
    return 0;
}

httpd_uri_t sse_uri = {
    .uri       = "/events",  // URL path
    .method    = HTTP_GET,
    .handler   = sse_handler,
    .user_ctx  = NULL
};

httpd_uri_t uri_json = {
    .uri = "/json",
    .method = HTTP_GET,
    .handler = get_json_handler,
    .user_ctx = NULL
};

httpd_uri_t uri_ldr = {
    .uri = "/ldr",
    .method = HTTP_GET,
    .handler = get_ldr_handler,
    .user_ctx = NULL
};

httpd_uri_t uri_dht_temp = {
    .uri = "/dht/temp",
    .method = HTTP_GET,
    .handler = get_dht_temp_handler,
    .user_ctx = NULL
};

httpd_uri_t uri_dht_humid = {
    .uri = "/dht/humid",
    .method = HTTP_GET,
    .handler = get_dht_humid_handler,
    .user_ctx = NULL
};

void initialize_http() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &sse_uri);
        httpd_register_uri_handler(server, &uri_ldr);
        httpd_register_uri_handler(server, &uri_dht_temp);
        httpd_register_uri_handler(server, &uri_dht_humid);
        httpd_register_uri_handler(server, &uri_json);
        ESP_LOGI(TAG_WEB, "STARTED HTTP");
    } else {
        ESP_LOGI(TAG_WEB, "DID NOT START HTTP");
    }
}