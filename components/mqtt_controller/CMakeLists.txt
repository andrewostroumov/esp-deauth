idf_component_register(
        SRCS "mqtt_controller.c"
        INCLUDE_DIRS "interface"
        REQUIRES "mqtt" "esp_discovery" "esp_macro"
)