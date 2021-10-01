#pragma once

#include "esp_err.h"
#include <stdbool.h>
#include "esp_nettap.h"
#include "esp_netif.h"
#include "bluetooth.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This file serves network related functions:
 * BNEP: PANU
 */

typedef struct {
    bd_addr_t remote_addr;
    uint16_t l2cap_psm;
    uint16_t uuid_dest;
} sbt_bnep_nap_target;

extern esp_netif_t *bnep_if;

/*
 * Bluetooth Network Encapsule Protocol client init
 *
 * Will init esp_netif as well
 */
esp_err_t sbt_bnep_panu_client_init(bd_addr_t mac);

/*
 * Bluetooth Network Encapsule Protocol client deinit
 *
 * Will deinit esp_netif as well
 */
esp_err_t sbt_bnep_panu_client_deinit();

/*
 * Act as panu client, connect to remote NAP service, return a netif handler
 */
esp_err_t sbt_bnep_panu_client_connect(sbt_bnep_nap_target *target);

/*
 * Act as panu client, disconnect from remote NAP service
 */
esp_err_t sbt_bnep_panu_client_disconnect(bd_addr_t addr);

/*
 * Get called when remote disconnected.
 * codes are executed in BTStack thread so do not block
 */
typedef void (*sbt_bnep_panu_client_disconnected_callback)(esp_err_t state);

/*
 * register callback for remote disconnect event.
 */
esp_err_t sbt_bnep_panu_client_register_disconnect_callback(sbt_bnep_panu_client_disconnected_callback cb);

#ifdef __cplusplus
}
#endif
