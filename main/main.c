#include <sys/cdefs.h>
#include <sys/queue.h>
#include <sys/cdefs.h>
#include <stdio.h>
#include <string.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include "wifi_controller.h"
#include "wifi_attack.h"
#include "mqtt_controller.h"
#include "esp_device.h"
#include "esp_blob.h"
#include "esp_discovery.h"

#define DEFAULT_BUF_SIZE 1024
#define SCAN_APS_NUM 5

static const char *TAG = "main";
static esp_device_t esp_device;
static EventGroupHandle_t wifi_event_group, mqtt_event_group;
static esp_timer_handle_t wifi_timer_handle;
static wifi_attack_config_t attack_config;

static const int WIFI_CONNECT_BIT = BIT0;
static const int WIFI_DISCONNECT_BIT = BIT1;

static const int MQTT_CONNECT_BIT = BIT0;
static const int MQTT_DISCONNECT_BIT = BIT1;
static const int MQTT_DATA_BIT = BIT2;
static const int MQTT_CONNECT_PUSH_BIT = BIT3;

static esp_discovery_t esp_discovery = {
        .esp_device = &esp_device
};

static esp_blob_t blob_config = {
        .namespace = "config",
        .key = "attack_t"
};

static wifi_config_t wifi_config = {
        .sta = {
                .ssid = CONFIG_WIFI_AP_SSID,
                .password = CONFIG_WIFI_AP_PASSWORD
        }
};

static void request_easy();
static void current_config_log();

static void attack_config_handle(const char *data) {
    wifi_attack_deserialize(&attack_config, data);
    current_config_log();
    esp_err_t err = esp_blob_set(&blob_config, (uint8_t *) &attack_config, sizeof(wifi_attack_config_t));

    if (err) {
        ESP_LOGE(TAG, "NVS attack config set error %x", err);
    }
}

static void wifi_timer_connect() {
    wifi_attack_reset();
    wifi_connect(&wifi_config);
}

_Noreturn static void task_mqtt_connected(void *arg) {
    while (true) {
        xEventGroupWaitBits(mqtt_event_group, MQTT_CONNECT_BIT, false, true, portMAX_DELAY);
        xEventGroupWaitBits(mqtt_event_group, MQTT_CONNECT_PUSH_BIT, true, true, portMAX_DELAY);
        mqtt_subscribe_command();
        mqtt_publish_available();
        mqtt_publish_discovery();
    }
}

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event) {
    char topic[DEFAULT_BUF_SIZE];
    char data[DEFAULT_BUF_SIZE];

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "[MQTT] Connected");
            xEventGroupSetBits(mqtt_event_group, MQTT_CONNECT_BIT | MQTT_CONNECT_PUSH_BIT);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "[MQTT] Disconnected");
            xEventGroupSetBits(mqtt_event_group, MQTT_DISCONNECT_BIT);
            break;
        case MQTT_EVENT_DATA:
            if ((xEventGroupGetBits(mqtt_event_group) | MQTT_DATA_BIT) == MQTT_DATA_BIT) {
                break;
            }

            strlcpy(topic, event->topic, event->topic_len + 1);
            strlcpy(data, event->data, event->data_len + 1);

            ESP_LOGI(TAG, "[MQTT] Data %s: %s", topic, data);

            if (strcmp(topic, esp_discovery.set_topic) == 0) {
                attack_config_handle(data);
                xEventGroupSetBits(mqtt_event_group, MQTT_DATA_BIT);
            }
            break;
        default:
            ESP_LOGV(TAG, "[MQTT] Event ID %d", event->event_id);
            break;
    }
    return ESP_OK;
}


static void wifi_event_handler(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data) {
    switch (id) {
//        case WIFI_EVENT_AP_PROBEREQRECVED:
//            ESP_LOGI(TAG, "Probe received");

        case WIFI_EVENT_STA_CONNECTED:
            ESP_LOGI(TAG, "Connected to %s", CONFIG_WIFI_AP_SSID);
            break;

        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, "Disconnected from %s", CONFIG_WIFI_AP_SSID);
            xEventGroupSetBits(wifi_event_group, WIFI_DISCONNECT_BIT);
            break;
    }
}

static void ip_event_handler(void *arg, esp_event_base_t base, int32_t id, void *event_data) {
    if (id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECT_BIT);
    }
}

_Noreturn static void task_wifi_connect(void *arg) {
    while (true) {
        xEventGroupWaitBits(wifi_event_group, WIFI_CONNECT_BIT, true, true, portMAX_DELAY);
        ESP_ERROR_CHECK(mqtt_connect());
    }
}

_Noreturn static void task_wifi_disconnect(void *arg) {
    while (true) {
        xEventGroupWaitBits(wifi_event_group, WIFI_DISCONNECT_BIT, true, true, portMAX_DELAY);
        if (esp_timer_is_active(wifi_timer_handle)) {
            continue;
        }

        request_easy();
    }
}

static const wifi_ap_record_t *wifi_attack_record() {
    esp_err_t err;

    if (!attack_config.state) {
        ESP_LOGI(TAG, "WIFI attack config state off");
        return NULL;
    }

    err = wifi_attack_validate(&attack_config);
    if (err) {
        ESP_LOGE(TAG, "WIFI attack config validate error: %d", err);
        return NULL;
    }

    return wifi_attack_scan_aps(attack_config.bssid);
}

static void request_easy() {
    const wifi_ap_record_t *ap_record = wifi_attack_record();

    if (ap_record == NULL) {
        wifi_connect(&wifi_config);
        return;
    }

    esp_timer_start_once(wifi_timer_handle, 60 * 1000000);

    wifi_attack_request(&attack_config, ap_record);
}

_Noreturn static void task_mqtt_data(void *arg) {
    esp_err_t err;
    char config_buffer[128];
    const wifi_ap_record_t *ap_record = NULL;

    while (true) {
        xEventGroupWaitBits(mqtt_event_group, MQTT_DATA_BIT, true, true, portMAX_DELAY);

        if (!attack_config.state) {
            wifi_attack_serialize(&attack_config, config_buffer);
            mqtt_publish_state(config_buffer);
            continue;
        }

        ap_record = NULL;

        for (int i = 0; i < SCAN_APS_NUM && ap_record == NULL; ++i) {
            ap_record = wifi_attack_record();
        }

        if (ap_record == NULL) {
            ESP_LOGI(TAG, "MQTT data handle ap_record NULL");

            attack_config.state = false;
            wifi_attack_serialize(&attack_config, config_buffer);
            mqtt_publish_state(config_buffer);

            xEventGroupClearBits(mqtt_event_group, MQTT_DATA_BIT);
            continue;
        }

        wifi_attack_serialize(&attack_config, config_buffer);
        mqtt_publish_state(config_buffer);

        // NOTE: start timer before Wi-Fi disconnect to rely on esp_timer_is_active
        esp_timer_start_once(wifi_timer_handle, 60 * 1000000);

        mqtt_disconnect();
        wifi_disconnect();

        wifi_attack_request(&attack_config, ap_record);

        xEventGroupClearBits(mqtt_event_group, MQTT_DATA_BIT);
    }
}

void create_timers() {
    const esp_timer_create_args_t wifi_timer_args = {
            .callback = &wifi_timer_connect
    };

    ESP_ERROR_CHECK(esp_timer_create(&wifi_timer_args, &wifi_timer_handle));
}

void create_connectivity() {
    mqtt_init(&esp_discovery, mqtt_event_handler);

    wifi_init();

    wifi_event_group = xEventGroupCreate();
    mqtt_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &ip_event_handler, NULL));
}

void nvs_get_blob_config() {
    esp_err_t err = esp_blob_get(&blob_config);

    if (err) {
        ESP_LOGE(TAG, "NVS attack config get error %x", err);
        return;
    }

    attack_config = *(wifi_attack_config_t *) blob_config.data;
    ESP_LOGI(TAG, "NVS has attack config");
}

void current_config_log() {
    char config_buffer[128];

    wifi_attack_serialize(&attack_config, config_buffer);
    ESP_LOGI(TAG, "Current config: %s", config_buffer);
}

void serial_log() {
    char *device_serial;
    esp_device_serial(&esp_device, &device_serial);

    ESP_LOGI(TAG, "Running %s %s", esp_device.manufacturer, esp_device.model);
    ESP_LOGI(TAG, "Serial %s", device_serial);
}

void app_main(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_device_init(&esp_device));
    ESP_ERROR_CHECK(esp_discovery_init(&esp_discovery));

    nvs_get_blob_config();

    serial_log();
    current_config_log();

    create_timers();
    create_connectivity();
    request_easy();

    xTaskCreate(task_wifi_connect, "task_wifi_connect", 2048, NULL, 0, NULL);
    xTaskCreate(task_wifi_disconnect, "task_wifi_disconnect", 2048, NULL, 0, NULL);
    xTaskCreate(task_mqtt_data, "task_mqtt_data", 2560, NULL, 0, NULL);
    xTaskCreate(task_mqtt_connected, "task_mqtt_connected", 2048, NULL, 0, NULL);
}

