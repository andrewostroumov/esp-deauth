idf_component_register(
        SRCS "wifi_attack.c" "broadcast.c" "rogueap.c"
        INCLUDE_DIRS "interface"
        PRIV_REQUIRES "wsl_bypasser" "wifi_controller" "json"
)