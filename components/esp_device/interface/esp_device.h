#ifndef ESP_DEVICE_H
#define ESP_DEVICE_H

#include "esp_err.h"

typedef struct {
    char *manufacturer;
    char *model;
    char *sw_version;
    char *idf_version;
    uint8_t *identifier;
    size_t identifier_length;
} esp_device_t;

esp_err_t esp_device_init(esp_device_t *device);

void esp_device_serial(esp_device_t *device, char **serial);

#endif