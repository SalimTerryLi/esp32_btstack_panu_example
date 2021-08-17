// Copyright 2019 Espressif Systems (Shanghai) PTE LTD
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

#include "esp_netif.h"
#include "esp_netif_lwip_internal.h"

#ifdef CONFIG_ESP_NETIF_TCPIP_LWIP

#include "esp_netif_lwip_tap.h"

//
// Purpose of this object is to define default network stack configuration
//  of basic types of interfaces using lwip network stack
//

static const struct esp_netif_netstack_config s_tap_netif_config = {
        .lwip = {
            .init_fn = tapif_init,
            .input_fn = tapif_input,
        }
};

const esp_netif_netstack_config_t *_g_esp_netif_netstack_default_tap      = &s_tap_netif_config;

#endif /*CONFIG_ESP_NETIF_TCPIP_LWIP*/
