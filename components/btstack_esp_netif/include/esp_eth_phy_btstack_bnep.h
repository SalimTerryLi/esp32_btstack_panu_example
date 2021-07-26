// Copyright 2019 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#pragma once

#include <stdbool.h>
#include <esp_netif.h>
#include "esp_netif_types.h"
#include "esp_eth_com.h"
#include "sdkconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ESP_ETH_PHY_ADDR_AUTO (-1)

/**
 * @brief Default configuration for Ethernet PHY object
 *
 */
#define ETH_PHY_DEFAULT_CONFIG()           \
    {                                      \
        .phy_addr = ESP_ETH_PHY_ADDR_AUTO, \
        .reset_timeout_ms = 100,           \
        .autonego_timeout_ms = 4000,       \
        .reset_gpio_num = 5,               \
    }

/**
* @brief Create a PHY instance of IP101
*
* @param[in] config: configuration of PHY
*
* @return
*      - instance: create PHY instance successfully
*      - NULL: create PHY instance failed because some error occurred
*/
esp_eth_phy_t *esp_eth_phy_new_btstack_bnep(const eth_phy_config_t *config);

#ifdef __cplusplus
}
#endif
