#include <string.h>
#include "cJSON.h"
#include "esp_discovery.h"

#define DISCOVERY_VALUE_TEMPLATE "{{ value_json.state }}"

static const char *DISCOVERY_TAG = "homeassistant/switch";
static const char *ZIGBEE_TAG = "espressif";
static const char *DEVICE_TAG = "switch";
static const char *DISCOVERY_PATH = "config";
static const char *DISCOVERY_PLATFORM = "mqtt";
static const char *SET_PATH = "set";
static const char *AVAILABILITY_PATH = "availability";

void esp_discovery_serialize(esp_discovery_t *discovery, char *data) {
    cJSON *root = cJSON_CreateObject();
    cJSON *device = cJSON_CreateObject();
    cJSON *identifiers = cJSON_CreateArray();
    cJSON *availability = cJSON_CreateArray();

    char *identifier;
    char *device_serial;
    esp_device_serial(discovery->esp_device, &device_serial);

    if (discovery->esp_device) {
        asprintf(&identifier, "%s_%s", discovery->esp_device->manufacturer, device_serial);
        cJSON_AddItemToArray(identifiers, cJSON_CreateString(identifier));

        cJSON_AddItemToObject(device, "identifiers", identifiers);
        cJSON_AddStringToObject(device, "manufacturer", discovery->esp_device->manufacturer);
        cJSON_AddStringToObject(device, "model", discovery->esp_device->model);
        cJSON_AddStringToObject(device, "name", device_serial);
        cJSON_AddStringToObject(device, "sw_version", discovery->esp_device->sw_version);
//        cJSON_AddStringToObject(device, "idf_version", discovery->esp_device->idf_version);

        cJSON_AddItemToObject(root, "device", device);
    }

    cJSON *availability_member = cJSON_CreateObject();
    cJSON_AddStringToObject(availability_member, "topic", discovery->status_topic);
    cJSON_AddItemToArray(availability, availability_member);
    cJSON_AddItemToObject(root, "availability", availability);

    cJSON_AddStringToObject(root, "platform", DISCOVERY_PLATFORM);
    cJSON_AddStringToObject(root, "name", discovery->name);
    cJSON_AddStringToObject(root, "unique_id", discovery->unique_id);
    cJSON_AddStringToObject(root, "value_template", DISCOVERY_VALUE_TEMPLATE);
    cJSON_AddStringToObject(root, "state_topic", discovery->state_topic);
    cJSON_AddStringToObject(root, "command_topic", discovery->set_topic);
    cJSON_AddStringToObject(root, "json_attributes_topic", discovery->attributes_topic);

    cJSON_AddBoolToObject(root, "retain", discovery->retain);
    cJSON_AddBoolToObject(root, "optimistic", discovery->optimistic);

    char *json = cJSON_PrintUnformatted(root);
    strcpy(data, json);
    cJSON_Delete(root);
}

esp_err_t esp_discovery_init(esp_discovery_t *discovery) {
    esp_discovery_ensure(discovery);

    discovery->optimistic = true;
    discovery->retain = true;

    return ESP_OK;
}

esp_err_t esp_discovery_ensure(esp_discovery_t *discovery) {
    char *device_serial;

    if (!discovery->esp_device) {
        return ESP_FAIL;
    }

    esp_device_serial(discovery->esp_device, &device_serial);

    asprintf(&discovery->name, "%s", device_serial);
    asprintf(&discovery->unique_id, "%s_%s_%s", device_serial, DEVICE_TAG, ZIGBEE_TAG);

    asprintf(&discovery->discovery_topic, "%s/%s/%s", DISCOVERY_TAG, device_serial, DISCOVERY_PATH);
    asprintf(&discovery->status_topic, "%s/%s/%s", ZIGBEE_TAG, device_serial, AVAILABILITY_PATH);
    asprintf(&discovery->set_topic, "%s/%s/%s", ZIGBEE_TAG, device_serial, SET_PATH);
    asprintf(&discovery->state_topic, "%s/%s", ZIGBEE_TAG, device_serial);

    discovery->attributes_topic = discovery->state_topic;

    free(device_serial);
    return ESP_OK;
}