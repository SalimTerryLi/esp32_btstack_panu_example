#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"
#include <stdbool.h>
#include "bluetooth.h"

/*
 * This file serves common BT functions like init, start and pair and so on
 */

/* init btstack (up to l2cap)
 * should only call once
 * name: device name that will be displayed if discoverable
 */
esp_err_t sbt_init(const char* name, uint32_t class_of_device);

/* set device discoverable */
esp_err_t sbt_set_discoverable(bool val);

/* so that device will receive SSP pair requests */
esp_err_t sbt_set_connectable(bool val);

esp_err_t sbt_get_local_mac(bd_addr_t addr);

/*
 * start BTStack in background and return when HCI is working
 */
esp_err_t sbt_start();

#ifdef __cplusplus
}
#endif
