#ifndef WIFI_CONTROLLER_H
#define WIFI_CONTROLLER_H

#include "esp_wifi_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

const int WIFI_BIT = BIT0;

void wifi_init();

void wifi_connect(wifi_config_t *wifi_config);

void wifi_disconnect();

void wifi_access_point(wifi_config_t *wifi_config);

void wifi_access_point_stop();

void wifi_set_ap_mac(const uint8_t *mac_ap);

void wifi_restore_ap_mac();

#endif