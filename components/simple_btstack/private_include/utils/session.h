#pragma once

#include <stdint.h>

#ifdef __cplusplus
//extern "C" {
#endif

#define SBT_SSP_TAG_CONFIRMATION_REQUEST 0x1

typedef uint8_t mac_addr[6];

/*
 * Create new session with given MAC, TAG and payload size
 *
 * return payload buffer, NULL if exists, or no mem
 */
void *create_session(mac_addr addr, uint32_t tag, uint32_t size);

/*
 * Delete session
 *
 * return 0 if success
 */
int delete_session(mac_addr addr, uint32_t tag);

/*
 * Find session with MAC and TAG
 *
 * return payload buffer
 */
void *find_session(mac_addr addr, uint32_t tag);

#ifdef __cplusplus
}
#endif
