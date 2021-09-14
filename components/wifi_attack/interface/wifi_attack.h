#ifndef WIFI_ATTACK_H
#define WIFI_ATTACK_H

#include "esp_err.h"
#include "esp_event.h"
#include "esp_wifi_types.h"

enum {
    WIFI_OK = 0,
    WIFI_ERR_METHOD_INVALID = 0x701,
    WIFI_ERR_BSSID_INVALID = 0x702,
    WIFI_ERR_TIMEOUT_INVALID = 0x703,
};

enum {
    WIFI_ATTACK_EVENT_REQUEST,
    WIFI_ATTACK_EVENT_RESET
};

typedef enum {
    WIFI_ATTACK_METHOD_NULL,
    WIFI_ATTACK_METHOD_BROADCAST,
    WIFI_ATTACK_METHOD_ROGUE_AP,
    WIFI_ATTACK_METHOD_COMBINE_ALL,
} wifi_attack_method_t;

typedef struct {
    uint8_t state;
    uint8_t method;
    uint8_t timeout;
    uint8_t bssid[6];
} wifi_attack_config_t;


const wifi_ap_record_t *wifi_attack_scan_aps(const uint8_t *bssid);

void wifi_attack_request(wifi_attack_config_t *config, const wifi_ap_record_t *ap_record);

void wifi_attack_reset();

void wifi_attack_serialize(wifi_attack_config_t *config, char *data);

void wifi_attack_deserialize(wifi_attack_config_t *config, const char *data);

esp_err_t wifi_attack_validate(wifi_attack_config_t *config);

#endif