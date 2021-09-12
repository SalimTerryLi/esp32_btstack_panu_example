#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "esp_log.h"
#include "btstack.h"
#include "gap.h"
#include "simple_btstack.h"
#include "sbt_common.h"
#include "sbt_debug.h"

static const char *TAG = "SBT_DEBUG";

static uint16_t packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(channel);
    UNUSED(size);

    bd_addr_t event_addr;

    switch (packet_type) {
        case HCI_EVENT_PACKET:
            ESP_LOGD(TAG, "HCI_EVENT_PACKET:");
            switch (hci_event_packet_get_type(packet)) {
                case HCI_EVENT_INQUIRY_COMPLETE:
                    ESP_LOGD(TAG, "\tHCI_EVENT_INQUIRY_COMPLETE");
                    break;
                case HCI_EVENT_INQUIRY_RESULT:
                    ESP_LOGD(TAG, "\tHCI_EVENT_INQUIRY_RESULT");
                    break;
                case HCI_EVENT_CONNECTION_COMPLETE:
                    // refer to Core_v5.3.pdf page 373
                    ESP_LOGD(TAG, "\tHCI_EVENT_CONNECTION_COMPLETE status: %d",
                             hci_event_connection_complete_get_status(packet));
                    break;
                case HCI_EVENT_CONNECTION_REQUEST:
                    ESP_LOGD(TAG, "\tHCI_EVENT_CONNECTION_REQUEST");
                    break;
                case HCI_EVENT_DISCONNECTION_COMPLETE:
                    ESP_LOGD(TAG, "\tHCI_EVENT_DISCONNECTION_COMPLETE");
                    break;
                case HCI_EVENT_AUTHENTICATION_COMPLETE:
                    ESP_LOGD(TAG, "\tHCI_EVENT_AUTHENTICATION_COMPLETE");
                    break;
                case HCI_EVENT_REMOTE_NAME_REQUEST_COMPLETE:
                    ESP_LOGD(TAG, "\tHCI_EVENT_REMOTE_NAME_REQUEST_COMPLETE");
                    break;
                case HCI_EVENT_ENCRYPTION_CHANGE:
                    ESP_LOGD(TAG, "\tHCI_EVENT_ENCRYPTION_CHANGE");
                    break;
                case HCI_EVENT_CHANGE_CONNECTION_LINK_KEY_COMPLETE:
                    ESP_LOGD(TAG, "\tHCI_EVENT_CHANGE_CONNECTION_LINK_KEY_COMPLETE");
                    break;
                case HCI_EVENT_MASTER_LINK_KEY_COMPLETE:
                    ESP_LOGD(TAG, "\tHCI_EVENT_MASTER_LINK_KEY_COMPLETE");
                    break;
                case HCI_EVENT_READ_REMOTE_SUPPORTED_FEATURES_COMPLETE:
                    ESP_LOGD(TAG, "\tHCI_EVENT_READ_REMOTE_SUPPORTED_FEATURES_COMPLETE");
                    break;
                case HCI_EVENT_READ_REMOTE_VERSION_INFORMATION_COMPLETE:
                    ESP_LOGD(TAG, "\tHCI_EVENT_READ_REMOTE_VERSION_INFORMATION_COMPLETE");
                    break;
                case HCI_EVENT_QOS_SETUP_COMPLETE:
                    ESP_LOGD(TAG, "\tHCI_EVENT_QOS_SETUP_COMPLETE");
                    break;
                case HCI_EVENT_COMMAND_COMPLETE:
                    ESP_LOGD(TAG, "\tHCI_EVENT_COMMAND_COMPLETE");
                    break;
                case HCI_EVENT_COMMAND_STATUS:
                    ESP_LOGD(TAG, "\tHCI_EVENT_COMMAND_STATUS");
                    break;
                case HCI_EVENT_HARDWARE_ERROR:
                    ESP_LOGD(TAG, "\tHCI_EVENT_HARDWARE_ERROR");
                    break;
                case HCI_EVENT_FLUSH_OCCURRED:
                    ESP_LOGD(TAG, "\tHCI_EVENT_FLUSH_OCCURRED");
                    break;
                case HCI_EVENT_ROLE_CHANGE:
                    ESP_LOGD(TAG, "\tHCI_EVENT_ROLE_CHANGE");
                    break;
                case HCI_EVENT_NUMBER_OF_COMPLETED_PACKETS:
                    ESP_LOGD(TAG, "\tHCI_EVENT_NUMBER_OF_COMPLETED_PACKETS");
                    break;
                case HCI_EVENT_MODE_CHANGE:
                    ESP_LOGD(TAG, "\tHCI_EVENT_MODE_CHANGE");
                    break;
                case HCI_EVENT_RETURN_LINK_KEYS:
                    ESP_LOGD(TAG, "\tHCI_EVENT_RETURN_LINK_KEYS");
                    break;
                case HCI_EVENT_PIN_CODE_REQUEST:
                    ESP_LOGD(TAG, "\tHCI_EVENT_PIN_CODE_REQUEST");
                    break;
                case HCI_EVENT_LINK_KEY_REQUEST:
                    ESP_LOGD(TAG, "\tHCI_EVENT_LINK_KEY_REQUEST");
                    break;
                case HCI_EVENT_LINK_KEY_NOTIFICATION:
                    ESP_LOGD(TAG, "\tHCI_EVENT_LINK_KEY_NOTIFICATION");
                    break;
                case HCI_EVENT_DATA_BUFFER_OVERFLOW:
                    ESP_LOGD(TAG, "\tHCI_EVENT_DATA_BUFFER_OVERFLOW");
                    break;
                case HCI_EVENT_MAX_SLOTS_CHANGED:
                    ESP_LOGD(TAG, "\tHCI_EVENT_MAX_SLOTS_CHANGED");
                    break;
                case HCI_EVENT_READ_CLOCK_OFFSET_COMPLETE:
                    ESP_LOGD(TAG, "\tHCI_EVENT_READ_CLOCK_OFFSET_COMPLETE");
                    break;
                case HCI_EVENT_CONNECTION_PACKET_TYPE_CHANGED:
                    ESP_LOGD(TAG, "\tHCI_EVENT_CONNECTION_PACKET_TYPE_CHANGED");
                    break;
                case HCI_EVENT_INQUIRY_RESULT_WITH_RSSI:
                    ESP_LOGD(TAG, "\tHCI_EVENT_INQUIRY_RESULT_WITH_RSSI");
                    break;
                case HCI_EVENT_READ_REMOTE_EXTENDED_FEATURES_COMPLETE:
                    ESP_LOGD(TAG, "\tHCI_EVENT_READ_REMOTE_EXTENDED_FEATURES_COMPLETE");
                    break;
                case HCI_EVENT_SYNCHRONOUS_CONNECTION_COMPLETE:
                    ESP_LOGD(TAG, "\tHCI_EVENT_SYNCHRONOUS_CONNECTION_COMPLETE");
                    break;
                case HCI_EVENT_EXTENDED_INQUIRY_RESPONSE:
                    ESP_LOGD(TAG, "\tHCI_EVENT_EXTENDED_INQUIRY_RESPONSE");
                    break;
                case HCI_EVENT_ENCRYPTION_KEY_REFRESH_COMPLETE:
                    ESP_LOGD(TAG, "\tHCI_EVENT_ENCRYPTION_KEY_REFRESH_COMPLETE");
                    break;
                case HCI_EVENT_IO_CAPABILITY_REQUEST:
                    ESP_LOGD(TAG, "\tHCI_EVENT_IO_CAPABILITY_REQUEST");
                    break;
                case HCI_EVENT_IO_CAPABILITY_RESPONSE:
                    ESP_LOGD(TAG, "\tHCI_EVENT_IO_CAPABILITY_RESPONSE");
                    break;
                case HCI_EVENT_USER_CONFIRMATION_REQUEST:
                    ESP_LOGD(TAG, "\tHCI_EVENT_USER_CONFIRMATION_REQUEST");
                    break;
                case HCI_EVENT_USER_PASSKEY_REQUEST:
                    ESP_LOGD(TAG, "\tHCI_EVENT_USER_PASSKEY_REQUEST");
                    break;
                case HCI_EVENT_REMOTE_OOB_DATA_REQUEST:
                    ESP_LOGD(TAG, "\tHCI_EVENT_REMOTE_OOB_DATA_REQUEST");
                    break;
                case HCI_EVENT_SIMPLE_PAIRING_COMPLETE:
                    ESP_LOGD(TAG, "\tHCI_EVENT_SIMPLE_PAIRING_COMPLETE %d",
                             hci_event_simple_pairing_complete_get_status(packet));
                    break;
                case HCI_EVENT_USER_PASSKEY_NOTIFICATION:
                    ESP_LOGD(TAG, "\tHCI_EVENT_USER_PASSKEY_NOTIFICATION");
                    break;
                case HCI_EVENT_KEYPRESS_NOTIFICATION:
                    ESP_LOGD(TAG, "\tHCI_EVENT_KEYPRESS_NOTIFICATION");
                    break;
                case HCI_EVENT_LE_META:
                    ESP_LOGD(TAG, "\tHCI_EVENT_LE_META");
                    break;
                case HCI_EVENT_VENDOR_SPECIFIC:
                    ESP_LOGD(TAG, "\tHCI_EVENT_VENDOR_SPECIFIC");
                    break;


                case HCI_EVENT_TRANSPORT_SLEEP_MODE:
                    ESP_LOGD(TAG, "\tHCI_EVENT_TRANSPORT_SLEEP_MODE");
                    break;
                case HCI_EVENT_TRANSPORT_READY:
                    ESP_LOGD(TAG, "\tHCI_EVENT_TRANSPORT_READY");
                    break;
                case HCI_EVENT_TRANSPORT_PACKET_SENT:
                    ESP_LOGD(TAG, "\tHCI_EVENT_TRANSPORT_PACKET_SENT");
                    break;
                case HCI_EVENT_SCO_CAN_SEND_NOW:
                    ESP_LOGD(TAG, "\tHCI_EVENT_SCO_CAN_SEND_NOW");
                    break;


                case 0x60:
                    ESP_LOGD(TAG, "\tDeprecated: Maybe HCI_Send_Keypress_Notification");
                    break;
                case 0x66:
                    ESP_LOGD(TAG, "\tHCI_Read_Flow_Control_Mode");
                    break;

                default:
                    ESP_LOGD(TAG, "\tunknown HCI event type 0x%.2x aka %u", hci_event_packet_get_type(packet), hci_event_packet_get_type(packet));
                    break;
            }
            break;
        case HCI_COMMAND_DATA_PACKET:
            ESP_LOGD(TAG, "HCI_COMMAND_DATA_PACKET:");
            break;
        case HCI_ACL_DATA_PACKET:
            ESP_LOGD(TAG, "HCI_ACL_DATA_PACKET:");
            break;
        case HCI_SCO_DATA_PACKET:
            ESP_LOGD(TAG, "HCI_SCO_DATA_PACKET:");
            break;

        default:
            ESP_LOGD(TAG, "unknown packet_type 0x%.2x aka %u", packet_type, packet_type);
            break;
    }
    return 0;
}

esp_err_t sbt_debugging_init() {
    xSemaphoreTake(sbt_packet_handler.mutex_lock, portMAX_DELAY);
    sbt_packet_handler.by_name.debug_handle = packet_handler;
    xSemaphoreGive(sbt_packet_handler.mutex_lock);
    return ESP_OK;
}
