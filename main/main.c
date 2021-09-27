/*
 * Copyright (C) 2020 BlueKitchen GmbH
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY MATTHIAS RINGWALD AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MATTHIAS
 * RINGWALD OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/*
 *  main.c
 *
 *  Minimal main application that initializes BTstack, prepares the example and enters BTstack's Run Loop.
 *
 *  If needed, you can create other threads. Please note that BTstack's API is not thread-safe and can only be
 *  called from BTstack timers or in response to its callbacks, e.g. packet handlers.
 */

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sbt_sdp.h"
#include "btstack_util.h"
#include "simple_btstack.h"
#include "bt_cod.h"
#include "sbt_pairing.h"
#include "sbt_debug.h"
#include "esp_log.h"

static const char* TAG = "APP_MAIN";

static sbt_pincode_response_t pincode_request(bd_addr_t addr) {
    sbt_pincode_response_t ret;
    strncpy(ret.pin, "0000", 5);
    return ret;
}

static void onSSPIncomingReq(bd_addr_t remote_addr, uint32_t pincode){
    vTaskDelay(pdMS_TO_TICKS(10000));
    sbt_pairing_comfirm_pincode(remote_addr, true);
}

static void onSSPFinish(bd_addr_t remote_addr, uint32_t status){
    ESP_LOGI(TAG, "Pairing returned 0x%.8x from %.2x:%.2x:%.2x:%.2x:%.2x:%.2x", status,
             remote_addr[0], remote_addr[1], remote_addr[2], remote_addr[3], remote_addr[4], remote_addr[5]);
}

int app_main(void){

    // optional: enable packet logger
    // hci_dump_open(NULL, HCI_DUMP_STDOUT);

    esp_log_level_set("SBT", ESP_LOG_DEBUG);
    esp_log_level_set("SBT_PAIRING", ESP_LOG_DEBUG);
    esp_log_level_set("SBT_SDP", ESP_LOG_DEBUG);
    sbt_init("ESP32 00:00:00:00:00:00", BT_COD_GEN(BT_COD_MAJOR_SERV_NETWORKING, BT_COD_MAJOR_DEV_COMPUTER, BT_COD_MAJOR_DEV_COMPUTER_MINOR_DEV_WEARABLE));
    ESP_LOGI(TAG, "BT init");
    sbt_set_discoverable(true);
    sbt_set_connectable(true);
    ESP_LOGI(TAG, "BT can be discovered and connected");
    sbt_pairing_pincode_request_register_callback(pincode_request);
    //sbt_debugging_init();
    sbt_pairing_enable_ssp(SBT_SSP_IO_CAP_DISPLAY_YES_NO, SBT_TRUST_MODEL_ASK_EVERYTIME, onSSPIncomingReq, onSSPFinish);
    sbt_start();
    ESP_LOGI(TAG, "BT running");

    vTaskDelay(pdMS_TO_TICKS(3000));

    ESP_LOGI(TAG, "SDP query");

    query_result_bnep_t query_result;
    bd_addr_t remote_addr;
    const char * remote_addr_string = "a4:6b:b6:3f:df:67";
    // convert string address to library-friendly one
    sscanf_bd_addr(remote_addr_string, remote_addr);
    if (sbt_sdp_query_nap(remote_addr, &query_result) != ESP_OK) {
        ESP_LOGI(TAG, "BNEP NAP not found");
    } else {
        ESP_LOGI(TAG, "BNEP NAP found");
    }

    return 0;
}
