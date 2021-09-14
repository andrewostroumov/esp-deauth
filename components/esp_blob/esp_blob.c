#include "esp_blob.h"
#include "esp_macro.h"
#include "nvs_flash.h"

esp_err_t esp_blob_get(esp_blob_t *esp_blob) {
    nvs_handle_t nvs;

    ESP_ERROR_RETURN(nvs_open(esp_blob->namespace, NVS_READONLY, &nvs));
    ESP_ERROR_RETURN(nvs_get_blob(nvs, esp_blob->key, NULL, &esp_blob->length));

    esp_blob->data = malloc(esp_blob->length);
    ESP_ERROR_RETURN(nvs_get_blob(nvs, esp_blob->key, esp_blob->data, &esp_blob->length));

    nvs_close(nvs);
    return ESP_OK;
}

esp_err_t esp_blob_set(esp_blob_t *esp_blob, uint8_t *data, size_t length) {
    nvs_handle_t nvs;

    ESP_ERROR_RETURN(nvs_open(esp_blob->namespace, NVS_READWRITE, &nvs));
    ESP_ERROR_RETURN(nvs_set_blob(nvs, esp_blob->key, data, length));

    nvs_commit(nvs);
    nvs_close(nvs);
    return ESP_OK;
}