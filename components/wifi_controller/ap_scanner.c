#include "ap_scanner.h"

#include <string.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_wifi.h"

static const char *TAG = "wifi_controller/ap_scanner";
static wifi_ap_records_t ap_records;

void wifi_scan_nearby_aps(const uint8_t *bssid) {
    ESP_LOGD(TAG, "Scanning nearby APs...");

    ap_records.count = CONFIG_WIFI_SCAN_MAX_AP;

    wifi_scan_config_t scan_config = {
            .ssid = NULL,
            .bssid = (uint8_t *) bssid,
            .channel = 0,
            .scan_type = WIFI_SCAN_TYPE_ACTIVE
    };

    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_records.count, ap_records.records));
    ESP_LOGI(TAG, "Found %u APs", ap_records.count);
    ESP_LOGD(TAG, "Scan done");
}

const wifi_ap_records_t *wifi_get_ap_records() {
    return &ap_records;
}

const wifi_ap_record_t *wifi_get_ap_record(const uint8_t *bssid) {
    ESP_LOGI(TAG, "Getting BSSID in nearby APs %.2x:%.2x:%.2x:%.2x:%.2x:%.2x", bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);

    for (int i = 0; i < ap_records.count; i++) {
        if (memcmp(ap_records.records[i].bssid, bssid, sizeof(ap_records.records[i].bssid)) == 0) {
            return &ap_records.records[i];
        }
    }

    return NULL;
}