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
#include <string.h>
#include <stdlib.h>
#include <sys/cdefs.h>
#include "esp_log.h"
#include "esp_eth.h"
#include "eth_phy_regs_struct.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_rom_gpio.h"
#include "esp_rom_sys.h"
#include "esp_eth_phy_btstack_bnep.h"

static const char *TAG = "ip101";
#define PHY_CHECK(a, str, goto_tag, ...)                                          \
    do                                                                            \
    {                                                                             \
        if (!(a))                                                                 \
        {                                                                         \
            ESP_LOGE(TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            goto goto_tag;                                                        \
        }                                                                         \
    } while (0)

/***************Vendor Specific Register***************/

typedef struct {
    esp_eth_phy_t parent;
    esp_eth_mediator_t *eth;
    int addr;
    eth_link_t link_status;
} phy_ip101_t;

static esp_err_t ip101_set_mediator(esp_eth_phy_t *phy, esp_eth_mediator_t *eth)
{
    PHY_CHECK(eth, "can't set mediator to null", err);
    phy_ip101_t *ip101 = __containerof(phy, phy_ip101_t, parent);
    ip101->eth = eth;
    return ESP_OK;
err:
    return ESP_ERR_INVALID_ARG;
}

static esp_err_t ip101_get_link(esp_eth_phy_t *phy)
{
    phy_ip101_t *ip101 = __containerof(phy, phy_ip101_t, parent);
    /* Updata information about link, speed, duplex */
    return ESP_OK;
}

static esp_err_t ip101_reset(esp_eth_phy_t *phy)
{
    phy_ip101_t *ip101 = __containerof(phy, phy_ip101_t, parent);
    ip101->link_status = ETH_LINK_DOWN;
    esp_eth_mediator_t *eth = ip101->eth;
    // reset
    return ESP_OK;
}

static esp_err_t ip101_reset_hw(esp_eth_phy_t *phy)
{
    phy_ip101_t *ip101 = __containerof(phy, phy_ip101_t, parent);
    // reset
    return ESP_OK;
}

static esp_err_t ip101_negotiate(esp_eth_phy_t *phy)
{
    phy_ip101_t *ip101 = __containerof(phy, phy_ip101_t, parent);
    esp_eth_mediator_t *eth = ip101->eth;

    return ESP_OK;
}

static esp_err_t ip101_pwrctl(esp_eth_phy_t *phy, bool enable)
{
    phy_ip101_t *ip101 = __containerof(phy, phy_ip101_t, parent);
    esp_eth_mediator_t *eth = ip101->eth;

    return ESP_OK;
}

static esp_err_t ip101_set_addr(esp_eth_phy_t *phy, uint32_t addr)
{
    phy_ip101_t *ip101 = __containerof(phy, phy_ip101_t, parent);
    ip101->addr = addr;
    return ESP_OK;
}

static esp_err_t ip101_get_addr(esp_eth_phy_t *phy, uint32_t *addr)
{
    PHY_CHECK(addr, "addr can't be null", err);
    phy_ip101_t *ip101 = __containerof(phy, phy_ip101_t, parent);
    *addr = ip101->addr;
    return ESP_OK;
    err:
    return ESP_ERR_INVALID_ARG;
}

static esp_err_t ip101_del(esp_eth_phy_t *phy)
{
    phy_ip101_t *ip101 = __containerof(phy, phy_ip101_t, parent);
    free(ip101);
    return ESP_OK;
}

static esp_err_t ip101_advertise_pause_ability(esp_eth_phy_t *phy, uint32_t ability)
{
    phy_ip101_t *ip101 = __containerof(phy, phy_ip101_t, parent);
    esp_eth_mediator_t *eth = ip101->eth;
    /* Set PAUSE function ability */

    return ESP_OK;
}

static esp_err_t ip101_init(esp_eth_phy_t *phy)
{
    phy_ip101_t *ip101 = __containerof(phy, phy_ip101_t, parent);
    esp_eth_mediator_t *eth = ip101->eth;
    // Detect PHY address

    return ESP_OK;
}

static esp_err_t ip101_deinit(esp_eth_phy_t *phy)
{
    /* Power off Ethernet PHY */
    return ESP_OK;
}

esp_eth_phy_t *esp_eth_phy_new_ip101(const eth_phy_config_t *config)
{
    PHY_CHECK(config, "can't set phy config to null", err);
    phy_ip101_t *ip101 = calloc(1, sizeof(phy_ip101_t));
    PHY_CHECK(ip101, "calloc ip101 failed", err);
    ip101->addr = config->phy_addr;
    ip101->link_status = ETH_LINK_DOWN;
    ip101->parent.reset = ip101_reset;
    ip101->parent.reset_hw = ip101_reset_hw;
    ip101->parent.init = ip101_init;
    ip101->parent.deinit = ip101_deinit;
    ip101->parent.set_mediator = ip101_set_mediator;
    ip101->parent.negotiate = ip101_negotiate;
    ip101->parent.get_link = ip101_get_link;
    ip101->parent.pwrctl = ip101_pwrctl;
    ip101->parent.get_addr = ip101_get_addr;
    ip101->parent.set_addr = ip101_set_addr;
    ip101->parent.advertise_pause_ability = ip101_advertise_pause_ability;
    ip101->parent.del = ip101_del;

    return &(ip101->parent);
err:
    return NULL;
}
