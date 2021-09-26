#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "btstack.h"
#include "gap.h"
#include "simple_btstack.h"
#include "sbt_common.h"
#include "sbt_bnep.h"
#include "utils/session.h"

static const char *TAG = "SBT_BNEP";


static uint16_t packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(channel);
    UNUSED(size);

    uint16_t consume_count = 0;

    bd_addr_t event_addr;
    uint32_t numeric;

    switch (packet_type) {
        case HCI_EVENT_PACKET:
            switch (hci_event_packet_get_type(packet)) {

                default:
                    break;
            }
            break;

        default:
            break;
    }
    return consume_count;
}

esp_err_t sbt_bnep_panu_client_init() {
    return ESP_OK;
}
