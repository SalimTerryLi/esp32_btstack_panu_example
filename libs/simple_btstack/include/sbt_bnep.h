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

/*
 * Get called when remote disconnected.
 * codes are executed in BTStack thread so do not block
 */
typedef void (*sbt_bnep_panu_client_disconnected_callback)();

/*
 * register callback for remote disconnect event.
 */
esp_err_t sbt_bnep_panu_client_register_disconnect_callback(sbt_bnep_panu_client_disconnected_callback cb);

#ifdef __cplusplus
}
#endif
