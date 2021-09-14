idf_component_register(
        SRCS "esp_discovery.c"
        INCLUDE_DIRS "interface"
        REQUIRES "esp_device"
        PRIV_REQUIRES  "json"
)