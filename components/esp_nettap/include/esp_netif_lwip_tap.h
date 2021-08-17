/*
 * SPDX-FileCopyrightText: 2015-2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#ifndef _TAP_LWIP_IF_H_
#define _TAP_LWIP_IF_H_

#include "lwip/err.h"

#ifdef __cplusplus
extern "C" {
#endif

err_t tapif_init(struct netif *netif);

void tapif_input(void *netif, void *buffer, size_t len, void *eb);

void netif_reg_addr_change_cb(void* cb);

#ifdef __cplusplus
}
#endif

#endif /*  _TAP_LWIP_IF_H_ */
