#ifndef AP_SCANNER_H
#define AP_SCANNER_H

#include "esp_wifi_types.h"

typedef struct {
    uint16_t count;
    wifi_ap_record_t records[CONFIG_WIFI_SCAN_MAX_AP];
} wifi_ap_records_t;

void wifi_scan_nearby_aps(const uint8_t *bssid);

const wifi_ap_records_t *wifi_get_ap_records();

const wifi_ap_record_t *wifi_get_ap_record();

#endif