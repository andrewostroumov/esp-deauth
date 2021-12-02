#include "mqtt_controller.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_discovery.h"
#include "esp_macro.h"

static const char *TAG = "mqtt_controller";
static esp_mqtt_client_handle_t client;
static esp_discovery_t discovery;
static char discovery_buffer[1024];

void mqtt_init(esp_discovery_t *esp_discovery, mqtt_event_callback_t event_handle) {
    const esp_mqtt_client_config_t mqtt_config = {
            .uri = CONFIG_MQTT_URI,
            .username = CONFIG_MQTT_USERNAME,
            .password = CONFIG_MQTT_PASSWORD,
            .event_handle = event_handle,
//            .lwt_topic = esp_discovery->status_topic,
//            .lwt_msg = STATUS_OFFLINE,
//            .lwt_qos = 1,
//            .lwt_retain = true,
            .keepalive = 10
    };

    client = esp_mqtt_client_init(&mqtt_config);
    memcpy(&discovery, esp_discovery, sizeof(esp_discovery_t));
}

esp_err_t mqtt_connect() {
    ESP_ERROR_RETURN(esp_mqtt_client_start(client));
    ESP_LOGI(TAG, "Connecting to %s...", CONFIG_MQTT_URI);
    return ESP_OK;
}

esp_err_t mqtt_disconnect() {
    ESP_ERROR_RETURN(esp_mqtt_client_stop(client));
    ESP_LOGI(TAG, "Disconnecting from %s...", CONFIG_MQTT_URI);
    return ESP_OK;
}

int mqtt_subscribe_command() {
    return esp_mqtt_client_subscribe(client, discovery.set_topic, 1);
}

int mqtt_publish_available() {
    return esp_mqtt_client_publish(client, discovery.status_topic, STATUS_ONLINE, 0, 1, true);
}

int mqtt_publish_unavailable() {
    return esp_mqtt_client_publish(client, discovery.status_topic, STATUS_OFFLINE, 0, 1, true);
}

int mqtt_publish_discovery() {
    esp_discovery_serialize(&discovery, discovery_buffer);
    ESP_LOGI(TAG, "Publish discovery %s: %s", discovery.discovery_topic, discovery_buffer);
    return esp_mqtt_client_publish(client, discovery.discovery_topic, discovery_buffer, 0, 1, true);
}

int mqtt_publish_state(const char *data) {
    ESP_LOGI(TAG, "Publish state %s: %s", discovery.state_topic, data);
    return esp_mqtt_client_publish(client, discovery.state_topic, data, 0, 1, true);
}
