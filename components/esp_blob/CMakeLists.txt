idf_component_register(
        SRCS "esp_blob.c"
        INCLUDE_DIRS "interface"
        PRIV_REQUIRES "nvs_flash" "esp_macro"
)