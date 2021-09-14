#ifndef ROGUEAP_H
#define ROGUEAP_H

#include "esp_wifi_types.h"

void rogueap_request(const wifi_ap_record_t *ap_record);

void rogueap_reset();

#endif
