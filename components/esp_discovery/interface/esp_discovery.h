#ifndef ESP_DISCOVERY_H
#define ESP_DISCOVERY_H

#include <esp_err.h>
#include <stdbool.h>
#include "esp_device.h"

typedef struct {
    esp_device_t *esp_device;
    char *name;
    char *unique_id;
    char *discovery_topic;
    char *status_topic;
    char *state_topic;
    char *set_topic;
    char *attributes_topic;
    bool retain;
    bool optimistic;
} esp_discovery_t;

void esp_discovery_serialize(esp_discovery_t *discovery, char *data);

esp_err_t esp_discovery_init(esp_discovery_t *discovery);

esp_err_t esp_discovery_ensure(esp_discovery_t *discovery);

#endif