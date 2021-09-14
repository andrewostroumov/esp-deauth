#ifndef MQTT_CONTROLLER_H
#define MQTT_CONTROLLER_H

#include "mqtt_client.h"
#include "esp_err.h"
#include "esp_discovery.h"

#define STATUS_ONLINE  "online"
#define STATUS_OFFLINE "offline"

void mqtt_init(esp_discovery_t *esp_discovery, mqtt_event_callback_t event_handle);

esp_err_t mqtt_connect();

esp_err_t mqtt_disconnect();

int mqtt_subscribe_command();

int mqtt_publish_available();

int mqtt_publish_unavailable();

int mqtt_publish_discovery();

int mqtt_publish_state(const char *data);

#endif