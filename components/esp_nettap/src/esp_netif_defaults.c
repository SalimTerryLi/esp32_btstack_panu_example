// Copyright 2015-2019 Espressif Systems (Shanghai) PTE LTD
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
#include "esp_tapif_defaults.h"
//
// Purpose of this module is to provide
//  - general esp-netif definitions of default objects for TAP
//  - default init / create functions for basic default interfaces
//

//
// Default configuration of TAP
//

const esp_netif_inherent_config_t _g_esp_netif_inherent_tap_config = ESP_NETIF_INHERENT_DEFAULT_TAP();

