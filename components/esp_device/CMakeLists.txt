idf_component_register(
        SRCS "esp_device.c"
        INCLUDE_DIRS "interface"
        PRIV_REQUIRES "esp_blob" "nvs_flash"
)