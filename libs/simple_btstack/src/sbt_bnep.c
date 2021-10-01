#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_check.h"
#include "btstack.h"
#include "gap.h"
#include "simple_btstack.h"
#include "sbt_common.h"
#include "sbt_bnep.h"
#include "utils/session.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_tapif_defaults.h"
#include "esp_nettap.h"

static const char *TAG = "SBT_BNEP";

/* only support single instance now */

static uint8_t remote_mac[6] = {};
static sbt_bnep_panu_client_disconnected_callback user_disconnect_cb = NULL;

static StaticSemaphore_t bin_sem_buffer_bnep_connect_notify;
static SemaphoreHandle_t bin_sem_bnep_connect_notify = NULL;
static bool connect_state = false;  // false: failed, true: succeed
static uint16_t bnep_cid = 0;
esp_netif_t *bnep_if = NULL;
static tap_netif_driver_t netif_drv;

static StaticSemaphore_t bin_sem_buffer_can_send;
static SemaphoreHandle_t bin_sem_can_send_notify = NULL;
static void *data_to_send = NULL;
static size_t data_to_send_len = 0;


static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(channel);
    UNUSED(size);

    bd_addr_t event_addr;

    switch (packet_type) {
        case HCI_EVENT_PACKET:
            switch (hci_event_packet_get_type(packet)) {
                case BNEP_EVENT_CHANNEL_OPENED:
                    if (bnep_event_channel_opened_get_status(packet)) {
                        ESP_LOGD(TAG, "BNEP channel open failed, status %02x", bnep_event_channel_opened_get_status(packet));
                        connect_state = false;
                        xSemaphoreGive(bin_sem_bnep_connect_notify);
                    } else {
                        bnep_cid    = bnep_event_channel_opened_get_bnep_cid(packet);
                        uint16_t uuid_source = bnep_event_channel_opened_get_source_uuid(packet);
                        uint16_t uuid_dest   = bnep_event_channel_opened_get_destination_uuid(packet);
                        uint16_t mtu         = bnep_event_channel_opened_get_mtu(packet);
                        bnep_event_channel_opened_get_remote_address(packet, event_addr);
                        ESP_LOGD(TAG, "BNEP connection open succeeded to %s source UUID 0x%04x dest UUID: 0x%04x, max frame size %u", bd_addr_to_str(event_addr), uuid_source, uuid_dest, mtu);
                        connect_state = true;
                        xSemaphoreGive(bin_sem_bnep_connect_notify);
                    }
                    break;

                case BNEP_EVENT_CHANNEL_TIMEOUT:
                    ESP_LOGW(TAG, "BNEP channel timeout! Channel will be closed");
                    break;

                case BNEP_EVENT_CHANNEL_CLOSED:
                    ESP_LOGD(TAG, "BNEP channel closed");
                    if (user_disconnect_cb) {
                        user_disconnect_cb(ESP_OK);
                    }
                    esp_netif_action_disconnected(bnep_if, NULL, 0, NULL);
                    esp_netif_action_stop(bnep_if, NULL, 0, NULL);
                    break;

                case BNEP_EVENT_CAN_SEND_NOW:
                    ESP_LOGD(TAG, "BNEP ethernet sending %u", data_to_send_len);
                    bnep_send(bnep_cid, data_to_send, data_to_send_len);
                    xSemaphoreGive(bin_sem_can_send_notify);

                    break;
                default:
                    break;
            }
            break;
        case BNEP_DATA_PACKET:
            // Write out the ethernet frame to the network interface
            ESP_LOGD(TAG, "BNEP ethernet received %u", size);
            esp_tap_process_incoming_packet(netif_drv, packet, size);
            break;

        default:
            break;
    }
}

static uint16_t packet_handler_wrap(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    packet_handler(packet_type, channel, packet, size);
    return 1;
}

static void sbt_bnep_register_packet_handle() {
    xSemaphoreTake(sbt_packet_handler.mutex_lock, portMAX_DELAY);
    sbt_packet_handler.by_name.bnep_handle = packet_handler_wrap;
    xSemaphoreGive(sbt_packet_handler.mutex_lock);
}
static void sbt_bnep_unregister_packet_handle() {
    xSemaphoreTake(sbt_packet_handler.mutex_lock, portMAX_DELAY);
    sbt_packet_handler.by_name.bnep_handle = NULL;
    xSemaphoreGive(sbt_packet_handler.mutex_lock);
}

static esp_err_t tap_transmit_btstack(void *data) {
    bnep_request_can_send_now_event(bnep_cid);
    return ESP_OK;
}

static esp_err_t tap_transmit(void* buffer, size_t len) {
    //ESP_LOGD(TAG, "BNEP want sending %u", len);
    data_to_send = buffer;
    data_to_send_len = len;
    exec_on_btstack(tap_transmit_btstack, NULL);
    xSemaphoreTake(bin_sem_can_send_notify, portMAX_DELAY);
    //ESP_LOGD(TAG, "BNEP sent %u", len);
    return ESP_OK;
}

esp_err_t sbt_bnep_panu_client_init(bd_addr_t mac) {
    esp_err_t ret = ESP_OK;
    bin_sem_bnep_connect_notify = xSemaphoreCreateBinaryStatic(&bin_sem_buffer_bnep_connect_notify);
    bin_sem_can_send_notify = xSemaphoreCreateBinaryStatic(&bin_sem_buffer_can_send);
    sbt_bnep_register_packet_handle();
    ESP_RETURN_ON_ERROR(esp_netif_init(), TAG, "esp_netif_init() failed");
    ESP_RETURN_ON_ERROR(esp_event_loop_create_default(), TAG, "esp_event_loop_create_default() failed");
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_TAP();
    bnep_if = esp_netif_new(&cfg);
    ESP_RETURN_ON_FALSE(bnep_if != NULL, ESP_ERR_INVALID_STATE, TAG, "esp_netif_new() failed");
    netif_drv = esp_tap_create_if_driver(tap_transmit);
    ESP_GOTO_ON_FALSE(netif_drv != NULL, ESP_ERR_INVALID_STATE, err, TAG, "esp_tap_create_if_driver() failed");
    ESP_GOTO_ON_ERROR(esp_netif_set_mac(bnep_if, mac), err, TAG, "esp_netif_set_mac() failed");
    ESP_GOTO_ON_ERROR(esp_netif_attach(bnep_if, netif_drv), err, TAG, "esp_netif_attach() failed");

    return ESP_OK;

err:
    if (netif_drv) {
        esp_tap_destroy_if_driver(netif_drv);
        netif_drv= NULL;
    }
    if (bnep_if) {
        esp_netif_destroy(bnep_if);
        bnep_if= NULL;
    }
    sbt_bnep_unregister_packet_handle();
    return ret;
}

esp_err_t sbt_bnep_panu_client_deinit() {
    sbt_bnep_unregister_packet_handle();
    esp_tap_destroy_if_driver(netif_drv);
    netif_drv= NULL;
    esp_netif_destroy(bnep_if);
    bnep_if= NULL;
    return ESP_OK;
}

static esp_err_t sbt_bnep_panu_client_connect_btstack(void *data) {
    sbt_bnep_nap_target *target = data;
    memcpy(remote_mac, target->remote_addr, 6);
    ESP_LOGD(TAG, "Try to connect to %.2x:%.2x:%.2x:%.2x:%.2x:%.2x psm %.4x uuid %.4x",
             target->remote_addr[0], target->remote_addr[1], target->remote_addr[2], target->remote_addr[3], target->remote_addr[4], target->remote_addr[5],
             target->l2cap_psm, target->uuid_dest);
    ESP_RETURN_ON_FALSE(bnep_connect(packet_handler, remote_mac, target->l2cap_psm, BLUETOOTH_SERVICE_CLASS_PANU, target->uuid_dest) == 0,
                        ESP_ERR_INVALID_STATE, TAG, "Failed to connect");
    return ESP_OK;
}

esp_err_t sbt_bnep_panu_client_connect(sbt_bnep_nap_target *target) {
    ESP_RETURN_ON_ERROR(exec_on_btstack(sbt_bnep_panu_client_connect_btstack, target), TAG, "bnep_connect() failed");
    xSemaphoreTake(bin_sem_bnep_connect_notify, portMAX_DELAY);
    ESP_LOGD(TAG, "Connection status: %u", connect_state);
    if (connect_state) {
        esp_netif_action_start(bnep_if, NULL, 0, NULL);
        esp_netif_action_connected(bnep_if, NULL, 0, NULL);

        return ESP_OK;
    } else {
        return ESP_ERR_INVALID_STATE;
    }
}

static esp_err_t sbt_bnep_panu_client_disconnect_btstack(void *data) {
    bnep_disconnect(data);
    return ESP_OK;
}

esp_err_t sbt_bnep_panu_client_disconnect(bd_addr_t addr) {
    return exec_on_btstack(sbt_bnep_panu_client_disconnect_btstack, addr);
}

esp_err_t sbt_bnep_panu_client_register_disconnect_callback(sbt_bnep_panu_client_disconnected_callback cb) {
    user_disconnect_cb = cb;
    return ESP_OK;
}