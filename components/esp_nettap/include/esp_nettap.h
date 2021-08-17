/*
 * SPDX-FileCopyrightText: 2015-2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdbool.h>
#include <esp_netif.h>
#include "esp_netif_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef esp_err_t (*tap_transmit_f)(void* buffer, size_t len);

/**
 * @brief Forward declaration of TAP interface handle
 */
typedef struct tap_netif_driver* tap_netif_driver_t;

/**
 * @brief Creates tap driver instance to be used with esp-netif
 *
 * @param fn callback function when there is data to be passed from network layer to link layer
 *
 * @return
 *  - pointer to wifi interface handle on success
 *  - NULL otherwise
 */
tap_netif_driver_t esp_tap_create_if_driver(tap_transmit_f fn);

/**
 * @brief Destroys tap driver instance
 *
 * @param h pointer to tap interface handle
 *
 */
void tap_destroy_if_driver(tap_netif_driver_t h);

/**
 * @brief Pass packets from link layer to network layer
 *
 * @param h pointer to tap interface handle
 * @param buffer data buffer
 * @param len data size
 *
 */
esp_err_t esp_tap_process_incoming_packet(tap_netif_driver_t h, void *buffer, size_t len);

#ifdef __cplusplus
}
#endif
