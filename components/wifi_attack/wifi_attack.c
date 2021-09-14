#include "wifi_attack.h"

#include "string.h"
#include "esp_err.h"
#include "esp_log.h"
#include "cJSON.h"

#include "ap_scanner.h"
#include "broadcast.h"
#include "rogueap.h"

static const char *STATE_ON = "ON";
static const char *STATE_OFF = "OFF";
static const char *METHOD_BROADCAST = "broadcast";
static const char *METHOD_ROGUE_AP = "rogue_ap";
static const char *METHOD_COMBINE_ALL = "combine_all";

static const char *TAG = "wifi_attack";
static wifi_attack_method_t running_method;
static char bssid_serialized[13];

static void copy_dispatch_method(wifi_attack_config_t *config) {
    running_method = config->method;
}

static void reset_dispatch_method() {
    running_method = WIFI_ATTACK_METHOD_NULL;
}

static void dispatch_reset() {
    switch (running_method) {
        case WIFI_ATTACK_METHOD_BROADCAST:
            broadcast_reset();
            break;
        case WIFI_ATTACK_METHOD_ROGUE_AP:
            rogueap_reset();
            break;
        case WIFI_ATTACK_METHOD_COMBINE_ALL:
            broadcast_reset();
            rogueap_reset();
            break;
        default:
            break;
    }

    reset_dispatch_method();
}

static void dispatch_request(wifi_attack_config_t *config, const wifi_ap_record_t *ap_record) {
    switch (config->method) {
        case WIFI_ATTACK_METHOD_BROADCAST:
            broadcast_request(config);
            break;
        case WIFI_ATTACK_METHOD_ROGUE_AP:
            rogueap_request(ap_record);
            break;
        case WIFI_ATTACK_METHOD_COMBINE_ALL:
            rogueap_request(ap_record);
            broadcast_request(config);
            break;
        default:
            break;
    }

    copy_dispatch_method(config);
}

const wifi_ap_record_t *wifi_attack_scan_aps(const uint8_t *bssid) {
    ESP_LOGD(TAG, "Scanning nearby APs");
    wifi_scan_nearby_aps(bssid);
    return wifi_get_ap_record(bssid);
}

void wifi_attack_request(wifi_attack_config_t *config, const wifi_ap_record_t *ap_record) {
    dispatch_reset();
    dispatch_request(config, ap_record);
}

void wifi_attack_reset() {
    dispatch_reset();
}

void wifi_attack_serialize(wifi_attack_config_t *config, char *data) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "state", config->state ? STATE_ON : STATE_OFF);
    cJSON_AddNumberToObject(root, "timeout", config->timeout);

    switch (config->method) {
        case WIFI_ATTACK_METHOD_BROADCAST:
            cJSON_AddStringToObject(root, "method", METHOD_BROADCAST);
            break;
        case WIFI_ATTACK_METHOD_ROGUE_AP:
            cJSON_AddStringToObject(root, "method", METHOD_ROGUE_AP);
            break;
        case WIFI_ATTACK_METHOD_COMBINE_ALL:
            cJSON_AddStringToObject(root, "method", METHOD_COMBINE_ALL);
            break;
        default:
            cJSON_AddNullToObject(root, "method");
            break;
    }

    memset(bssid_serialized, ':', sizeof(config->bssid));

    for (size_t count = 0; count < sizeof(config->bssid) / sizeof(*config->bssid); count++) {
        if (count < sizeof(config->bssid) / sizeof(*config->bssid) - 1) {
            sprintf(bssid_serialized + count * 3, "%.2hhx:", config->bssid[count]);
        } else {
            sprintf(bssid_serialized + count * 3, "%.2hhx", config->bssid[count]);
        }
    }

    cJSON_AddStringToObject(root, "bssid", bssid_serialized);

    char *json = cJSON_PrintUnformatted(root);
    strcpy(data, json);
    cJSON_Delete(root);
}

// NOTE: MQTT ATTRIBUTES
// state: on / off (string)
// method: broadcast, rogue_ap, combine_all (string)
// bssid: ff:dd:cc:dd:ee:ff (string)
// timeout: 1 (int)
void wifi_attack_deserialize(wifi_attack_config_t *config, const char *data) {
    if (strcmp(data, STATE_ON) == 0) {
        config->state = true;
        return;
    }

    if (strcmp(data, STATE_OFF) == 0) {
        config->state = false;
        return;
    }

    cJSON *root = cJSON_Parse(data);

    if (!root) {
        return;
    }

    cJSON *state = cJSON_GetObjectItem(root, "state");
    if (state) {
        config->state = strcmp(state->valuestring, STATE_ON) == 0;
    }

    cJSON *timeout = cJSON_GetObjectItem(root, "timeout");
    if (timeout) {
        config->timeout = timeout->valueint;
    }

    cJSON *method = cJSON_GetObjectItem(root, "method");
    if (method) {
        if (strcmp(method->valuestring, METHOD_BROADCAST) == 0) {
            config->method = WIFI_ATTACK_METHOD_BROADCAST;
        } else if (strcmp(method->valuestring, METHOD_ROGUE_AP) == 0) {
            config->method = WIFI_ATTACK_METHOD_ROGUE_AP;
        } else if (strcmp(method->valuestring, METHOD_COMBINE_ALL) == 0) {
            config->method = WIFI_ATTACK_METHOD_COMBINE_ALL;
        }
    }

    cJSON *bssid = cJSON_GetObjectItem(root, "bssid");
    if (bssid) {
        const char *pos = bssid->valuestring;

        for (size_t count = 0; count < sizeof(config->bssid) / sizeof(*config->bssid) && count * 3 < strlen(bssid->valuestring); count++) {
            sscanf(pos + count * 3, "%2hhx", &config->bssid[count]);
        }
    }

    cJSON_Delete(root);
}

esp_err_t wifi_attack_validate(wifi_attack_config_t *config) {
    if (config->method == WIFI_ATTACK_METHOD_NULL) {
        return WIFI_ERR_METHOD_INVALID;
    }

    if (config->timeout == 0) {
        return WIFI_ERR_TIMEOUT_INVALID;
    }

    return WIFI_OK;
}