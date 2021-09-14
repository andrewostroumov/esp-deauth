#ifndef ESP_MACRO_H
#define ESP_MACRO_H

#define ESP_ERROR_RETURN(x) ({      \
        esp_err_t esp_err = (x);    \
        if (esp_err != ESP_OK) {    \
            return esp_err;         \
        }                           \
})
#endif