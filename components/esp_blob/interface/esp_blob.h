#include "esp_err.h"

typedef struct {
    char *namespace;
    char *key;
    uint8_t *data;
    size_t length;
} esp_blob_t;

esp_err_t esp_blob_get(esp_blob_t *esp_blob);

esp_err_t esp_blob_set(esp_blob_t *esp_blob, uint8_t *data, size_t length);