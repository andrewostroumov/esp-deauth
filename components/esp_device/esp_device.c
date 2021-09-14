#include "esp_device.h"
#include "esp_blob.h"

#include "esp_system.h"
#include "nvs_flash.h"

#define DEVICE_MANUFACTURER "espressif"
#define DEVICE_ID_NVS_NS "device"
#define DEVICE_ID_NVS_KEY "id"
#define DEVICE_ID_NVS_LENGTH 8

static esp_blob_t esp_serial = {
        .namespace = DEVICE_ID_NVS_NS,
        .key = DEVICE_ID_NVS_KEY,
};

static esp_err_t esp_serial_ensure() {
    esp_err_t err;
    err = esp_blob_get(&esp_serial);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        return err;
    }

    if (esp_serial.length) {
        return ESP_OK;
    }

    esp_serial.length = DEVICE_ID_NVS_LENGTH;
    esp_serial.data = malloc(DEVICE_ID_NVS_LENGTH);
    esp_fill_random(esp_serial.data, esp_serial.length);

    return esp_blob_set(&esp_serial, esp_serial.data, esp_serial.length);
}

static void esp_device_model(esp_device_t *device) {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    asprintf(&device->model, "%s with %d CPU core(s) silicon revision %d (WiFi%s%s%s)",
             CONFIG_IDF_TARGET,
             chip_info.cores,
             chip_info.revision,
             (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
             (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "",
             (chip_info.features & CHIP_FEATURE_IEEE802154) ? "/IEEE_802.15.4" : "");
}

static void esp_device_identifier(esp_device_t *device) {
    device->identifier = esp_serial.data;
    device->identifier_length = esp_serial.length;
}

esp_err_t esp_device_init(esp_device_t *device) {
    device->manufacturer = DEVICE_MANUFACTURER;
    device->idf_version = (char *) esp_get_idf_version();

    esp_err_t err = esp_serial_ensure();
    if (err != ESP_OK) {
        return err;
    }

    esp_device_model(device);
    esp_device_identifier(device);

    return ESP_OK;
}

void esp_device_serial(esp_device_t *device, char **serial) {
    *serial = malloc(device->identifier_length * 2 + 3);
    char *p = *serial;
    p += sprintf(p, "0x");

    for (size_t i = 0; i < device->identifier_length; ++i) {
        p += sprintf(p, "%02x", device->identifier[i]);
    }
}
