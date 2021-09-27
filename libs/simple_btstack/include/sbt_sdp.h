#pragma once

#include "esp_err.h"
#include <stdbool.h>
#include "bluetooth.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This file serves Service Discovery Protocol
 */

/*
 * definitions of SDP query targets
 */
#define SBT_SDP_QUERY_BNEP_NAP BLUETOOTH_SERVICE_CLASS_NAP

typedef struct {
    uint16_t sdp_bnep_l2cap_psm;
    uint16_t sdp_bnep_version;
    uint32_t sdp_bnep_remote_uuid;
} query_result_bnep_t;

esp_err_t sbt_sdp_query_nap(bd_addr_t remote, query_result_bnep_t *result);

#ifdef __cplusplus
}
#endif
