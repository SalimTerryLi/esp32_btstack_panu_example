#pragma once

#include "esp_err.h"
#include <stdbool.h>
#include "bluetooth.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This file serves pairing related functions:
 * SSP, PINCODE
 */

/*
 * pincode response that will be sent back to remote host
 */
typedef struct sbt_pincode_response_s {
    char pin[16];
} sbt_pincode_response_t;

/*
 * Being called when remote host is requesting pincode.
 * remote_addr contains remote addr
 * should return sbt_pincode_response_t
 * should not do blocking here
 */
typedef sbt_pincode_response_t (*sbt_pincode_request_callback_t)(bd_addr_t remote_addr);

/*
 * Accept the pairing request only if pincode received is the same as remote's
 *
 * block until pairing finished
 */
esp_err_t sbt_pairing_comfirm_pincode(bd_addr_t mac, bool accept);

/*
 * provide pincode that is displayed on the remote
 * pincode > 999999 will deny the request
 *
 * block until pairing finished
 */
esp_err_t sbt_pairing_provide_pincode(bd_addr_t mac, uint32_t pincode);

/*
 * Get called when ssp pairing request is sent from remote
 */
typedef void (*sbt_ssp_request_callback_t)(bd_addr_t remote_addr, uint32_t pincode);

/*
 * Get called when incoming SSP complete
 *
 * status = 0 means success
 */
typedef void (*sbt_ssp_result_callback_t)(bd_addr_t remote_addr, uint32_t status);

/*
 * Register a callback that get called when other device try to get a pincode from this device
 * put argv callback to NULL means clear the registered callback
 *
 * It's a very old fashion so that most devices will just use SSP to pair if enabled. Only works if
 * SSP is disabled. Otherwise, other device usually won't request pincode at all.
 *
 * But seems not working at allll
 */
esp_err_t sbt_pairing_pincode_request_register_callback(sbt_pincode_request_callback_t callback);

typedef enum sbt_ssp_io_capability_s {
    SBT_SSP_IO_CAP_DISPLAY_ONLY = 0,        /*!< remote should confirm the code displayed on this device */
    SBT_SSP_IO_CAP_DISPLAY_YES_NO,          /*!< both remote and this device need to confirm the pin code */
    SBT_SSP_IO_CAP_KEYBOARD_ONLY,           /*!< this device should input what remote host is displayed */
    SBT_SSP_IO_CAP_NO_INPUT_NO_OUTPUT,      /*!< auto accept, no MITM protection */
    SBT_SSP_IO_CAP_UNKNOWN,                 /*!< ??? */
} sbt_ssp_io_capability_t;

typedef enum sbt_ssp_trust_model_s {
    SBT_TRUST_MODEL_ASK_EVERYTIME = 0,      /*!< remote should ask for user every time */
    SBT_TRUST_MODEL_ASK_FOR_NEW_SERVICE,    /*!< remote should ask for user only new service */
    SBT_TRUST_MODEL_ALWAYS_TRUST,           /*!< remote should always trust connections */
} sbt_ssp_trust_model_t;

/*
 * BT v2.1 feature, Secure Simple Pairing. Must be called in configure stage
 *
 * can_auto_accept is used to tell remote host whether this device may automatically accept or not
 * automatic accept will cause MITM issue so may not be trusted by remote in certain conditions
 *
 * io_cap describes user interact possibility of this device. NO_INPUT_NO_OUTPUT usually means will auto accept connections
 * otherwise it is possible to provide MITM protection.
 *
 * result_cb can be NULL
 *
 * trust is not implemented
 */
esp_err_t sbt_pairing_enable_ssp(sbt_ssp_io_capability_t io_cap, sbt_ssp_trust_model_t trust,
                                 sbt_ssp_request_callback_t req_cb, sbt_ssp_result_callback_t result_cb);

#ifdef __cplusplus
}
#endif
