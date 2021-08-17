/*
 * SPDX-FileCopyrightText: 2015-2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <esp_log.h>
#include "esp_nettap.h"

typedef struct tap_interface {
    tap_transmit_f tx_func;
} tap_interface_t;

/**
 * @brief BNEP netif driver structure
 */
typedef struct tap_netif_driver {
    esp_netif_driver_base_t base;
    tap_interface_t tap_if;
}* tap_netif_driver_t;

static const char* TAG = "tap_netif";

static esp_err_t tap_transmit(void *h, void *buffer, size_t len)
{
    tap_netif_driver_t driver = h;
    return driver->tap_if.tx_func(buffer, len);
}

static esp_err_t tap_driver_start(esp_netif_t * esp_netif, void * args)
{
    tap_netif_driver_t driver = args;
    driver->base.netif = esp_netif;
    esp_netif_driver_ifconfig_t driver_ifconfig = {
            .handle =  driver,
            .transmit = tap_transmit,
            .driver_free_rx_buffer = NULL
    };

    return esp_netif_set_driver_config(esp_netif, &driver_ifconfig);
}

tap_netif_driver_t esp_tap_create_if_driver(tap_transmit_f fn)
{
    tap_netif_driver_t driver = calloc(1, sizeof(struct tap_netif_driver));
    if (driver == NULL) {
        ESP_LOGE(TAG, "No memory to create a wifi interface handle");
        return NULL;
    }
    driver->base.post_attach = tap_driver_start;
    driver->tap_if.tx_func = fn;
    return driver;
}

void esp_tap_destroy_if_driver(tap_netif_driver_t h)
{
    free(h);
}

esp_err_t esp_tap_process_incoming_packet(tap_netif_driver_t h, void *buffer, size_t len){
    return esp_netif_receive(h->base.netif, buffer, len, NULL);
}
