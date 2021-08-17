// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _ESP_TAPIF_DEFAULTS_H
#define _ESP_TAPIF_DEFAULTS_H

#include "esp_compiler.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// Macros to assemble master configs with partial configs from netif, stack and driver
//

#define ESP_NETIF_INHERENT_DEFAULT_TAP() \
    {   \
        .flags = (esp_netif_flags_t)(ESP_NETIF_DHCP_CLIENT | ESP_NETIF_FLAG_GARP | ESP_NETIF_FLAG_EVENT_IP_MODIFIED), \
        ESP_COMPILER_DESIGNATED_INIT_AGGREGATE_TYPE_EMPTY(mac) \
        ESP_COMPILER_DESIGNATED_INIT_AGGREGATE_TYPE_EMPTY(ip_info) \
        .get_ip_event = IP_EVENT_ETH_GOT_IP,    \
        .lost_ip_event = IP_EVENT_ETH_LOST_IP,   \
        .if_key = "TAP_DEF",  \
        .if_desc = "soft_tap",    \
        .route_prio = 5      \
};

/**
* @brief  Default configuration reference of TAP client
*/
#define ESP_NETIF_DEFAULT_TAP()                       \
    {                                                  \
        .base = ESP_NETIF_BASE_DEFAULT_TAP,           \
        .driver = NULL,                                \
        .stack = ESP_NETIF_NETSTACK_DEFAULT_TAP,      \
    }

/**
 * @brief  Default base config (esp-netif inherent) of TAP interface
 */
#define ESP_NETIF_BASE_DEFAULT_TAP             &_g_esp_netif_inherent_tap_config

#define ESP_NETIF_NETSTACK_DEFAULT_TAP          _g_esp_netif_netstack_default_tap

//
// Include default network stacks configs
//  - Network stack configurations are provided in a specific network stack
//      implementation that is invisible to user API
//  - Here referenced only as opaque pointers
//
extern const esp_netif_netstack_config_t *_g_esp_netif_netstack_default_tap;

//
// Include default common configs inherent to esp-netif
//  - These inherent configs are defined in esp_netif_defaults.c and describe
//    common behavioural patterns for common interfaces such as STA, AP, ETH, PPP
//
extern const esp_netif_inherent_config_t _g_esp_netif_inherent_tap_config;

#ifdef __cplusplus
}
#endif

#endif //_ESP_TAPIF_DEFAULTS_H
