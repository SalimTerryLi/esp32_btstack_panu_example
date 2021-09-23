#pragma once

#include "esp_err.h"
#include <stdbool.h>
#include "bluetooth.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This file serves network related functions:
 * BNEP: PANU
 */

/*
 * Bluetooth Network Encapsule Protocol client init
 *
 * Will init esp_netif as well
 */
esp_err_t sbt_bnep_panu_client_init();

/*
 * Bluetooth Network Encapsule Protocol client deinit
 *
 * Will deinit esp_netif as well
 */
esp_err_t sbt_bnep_panu_client_deinit();

/*
 * Act as panu client, connect to remote NAP service
 */
esp_err_t sbt_bnep_panu_client_connect();

/*
 * Act as panu client, disconnect from remote NAP service
 */
esp_err_t sbt_bnep_panu_client_disconnect();

#ifdef __cplusplus
}
#endif
