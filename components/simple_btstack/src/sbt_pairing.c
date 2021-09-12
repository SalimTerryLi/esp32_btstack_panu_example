#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "btstack.h"
#include "gap.h"
#include "simple_btstack.h"
#include "sbt_common.h"
#include "sbt_pairing.h"
#include "utils/session.h"

static const char *TAG = "SBT_PAIRING";

static sbt_pincode_request_callback_t user_pincode_callback = NULL;

static sbt_ssp_io_capability_t user_io_cap = SBT_SSP_IO_CAP_UNKNOWN;
static sbt_ssp_request_callback_t user_ssp_request_callback = NULL;
static sbt_ssp_result_callback_t user_ssp_result_callback = NULL;

typedef struct sbt_incomming_pairing_data_S {
    bd_addr_t remote_addr;
    uint32_t pincode;
    TaskHandle_t task;
} sbt_incomming_pairing_data_t;
static void sbt_incomming_pairing_task(void* arg) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    ESP_LOGD(TAG, "new pairing task started");
    sbt_incomming_pairing_data_t *data = (sbt_incomming_pairing_data_t*) arg;

    if (data->task != xTaskGetCurrentTaskHandle()){
        ESP_LOGE(TAG, "CRITICAL: DIFFERENT TASK");
        goto exit;
    }

    user_ssp_request_callback(data->remote_addr, data->pincode);

    // wait for HCI_EVENT_SIMPLE_PAIRING_COMPLETE
    uint32_t pairing_result;
    xTaskNotifyWait(0, ULONG_MAX, &pairing_result, portMAX_DELAY);
    user_ssp_result_callback(data->remote_addr, pairing_result);
exit:
    if(delete_session(data->remote_addr, SBT_SSP_TAG_CONFIRMATION_REQUEST) != 0){
        ESP_LOGE(TAG, "failed to delete session");
    }
    ESP_LOGD(TAG, "pairing task exited");
    vTaskDelete(NULL);
}

static uint16_t packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(channel);
    UNUSED(size);

    uint16_t consume_count = 0;

    bd_addr_t event_addr;
    uint32_t numeric;

    switch (packet_type) {
        case HCI_EVENT_PACKET:
            switch (hci_event_packet_get_type(packet)) {
                case HCI_EVENT_PIN_CODE_REQUEST:
                    ++consume_count;
                    hci_event_pin_code_request_get_bd_addr(packet, event_addr);
                    ESP_LOGD(TAG, "PINCODE request from %.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
                             event_addr[0], event_addr[1], event_addr[2], event_addr[3], event_addr[4], event_addr[5]);
                    if (user_pincode_callback != NULL){
                        sbt_pincode_response_t response;
                        response = user_pincode_callback(event_addr);
                        gap_pin_code_response(event_addr, response.pin);
                        ESP_LOGD(TAG, "PINCODE responsed with %s", response.pin);
                    }
                    break;

                case HCI_EVENT_USER_CONFIRMATION_REQUEST:
                    hci_event_user_confirmation_request_get_bd_addr(packet, event_addr);
                    numeric = hci_event_user_confirmation_request_get_numeric_value(packet);
                    ESP_LOGD(TAG, "SSP request from %.2x:%.2x:%.2x:%.2x:%.2x:%.2x with code %u",
                             event_addr[0], event_addr[1], event_addr[2], event_addr[3], event_addr[4], event_addr[5], numeric);
                    if (user_io_cap == SBT_SSP_IO_CAP_DISPLAY_YES_NO) {
                        sbt_incomming_pairing_data_t *pairing_data = create_session(event_addr, SBT_SSP_TAG_CONFIRMATION_REQUEST, sizeof(sbt_incomming_pairing_data_t));
                        if (pairing_data == NULL) {
                            ESP_LOGE(TAG, "Failed to create session");
                            break;
                        }
                        memcpy(pairing_data->remote_addr, event_addr, 6);
                        pairing_data->pincode = numeric;
                        xTaskCreate(sbt_incomming_pairing_task, "btstack_ssp_req", 2048, pairing_data,
                                    uxTaskPriorityGet(xTaskGetCurrentTaskHandle()), &pairing_data->task);
                        xTaskNotifyGive(pairing_data->task);
                    } else {
                        gap_ssp_confirmation_response(event_addr);
                    }
                    break;
                case HCI_EVENT_KEYPRESS_NOTIFICATION:
                    break;

                case HCI_EVENT_USER_PASSKEY_REQUEST:
                    hci_event_user_passkey_request_get_bd_addr(packet, event_addr);
                    if (user_io_cap == SBT_SSP_IO_CAP_KEYBOARD_ONLY){
                        sbt_incomming_pairing_data_t *pairing_data = create_session(event_addr, SBT_SSP_TAG_CONFIRMATION_REQUEST, sizeof(sbt_incomming_pairing_data_t));
                        memcpy(pairing_data->remote_addr, event_addr, 6);
                        xTaskCreate(sbt_incomming_pairing_task, "btstack_ssp_req", 2048, pairing_data,
                                    uxTaskPriorityGet(xTaskGetCurrentTaskHandle()), &pairing_data->task);
                        xTaskNotifyGive(pairing_data->task);
                    }
                    break;
                case HCI_EVENT_REMOTE_OOB_DATA_REQUEST:
                    break;
                case HCI_EVENT_USER_PASSKEY_NOTIFICATION:
                    numeric = hci_event_user_passkey_notification_get_numeric_value(packet);
                    ESP_LOGD(TAG, "HCI_EVENT_USER_PASSKEY_NOTIFICATION numeric: %u", numeric);
                    break;

                case HCI_EVENT_SIMPLE_PAIRING_COMPLETE:
                    hci_event_simple_pairing_complete_get_bd_addr(packet, event_addr);
                    uint32_t pairing_result = hci_event_simple_pairing_complete_get_status(packet);
                    if (pairing_result == 0){
                        ESP_LOGD(TAG, "Pairing complete with %.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
                                 event_addr[0], event_addr[1], event_addr[2], event_addr[3], event_addr[4], event_addr[5]);
                    } else {
                        ESP_LOGD(TAG, "Pairing failed with %.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
                                 event_addr[0], event_addr[1], event_addr[2], event_addr[3], event_addr[4], event_addr[5]);
                    }
                    sbt_incomming_pairing_data_t *pairing_data = find_session(event_addr, SBT_SSP_TAG_CONFIRMATION_REQUEST);
                    if (pairing_data != NULL){
                        xTaskGenericNotify(pairing_data->task, pairing_result, eSetValueWithOverwrite, NULL);
                    }
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }
    return consume_count;
}

static uint16_t packet_handle_ref_count = 0;
static void sbt_pairing_register_packet_handle() {
    xSemaphoreTake(sbt_packet_handler.mutex_lock, portMAX_DELAY);
    sbt_packet_handler.by_name.pairing_handle = packet_handler;
    ++packet_handle_ref_count;
    xSemaphoreGive(sbt_packet_handler.mutex_lock);
}
static void sbt_pairing_unregister_packet_handle() {
    xSemaphoreTake(sbt_packet_handler.mutex_lock, portMAX_DELAY);
    --packet_handle_ref_count;
    if (packet_handle_ref_count == 0) {
        sbt_packet_handler.by_name.pairing_handle = NULL;
    }
    xSemaphoreGive(sbt_packet_handler.mutex_lock);
}

esp_err_t sbt_pairing_pincode_request_register_callback(sbt_pincode_request_callback_t callback) {
    if (callback != NULL) {
        if (user_pincode_callback == NULL) {
            user_pincode_callback = callback;
            sbt_pairing_register_packet_handle();
            return ESP_OK;
        } else {
            ESP_LOGE(TAG, "pincode callback already registered");
            return ESP_ERR_INVALID_STATE;
        }
    }
    sbt_pairing_unregister_packet_handle();
    sbt_packet_handler.by_name.pairing_handle = NULL;
    return ESP_OK;
}

esp_err_t sbt_pairing_enable_ssp(sbt_ssp_io_capability_t io_cap, sbt_ssp_trust_model_t trust,
                                 sbt_ssp_request_callback_t req_cb, sbt_ssp_result_callback_t result_cb) {
    gap_ssp_set_enable(1);
    gap_ssp_set_io_capability(io_cap);
    uint16_t mitm_req = 1;
    if (io_cap == SBT_SSP_IO_CAP_UNKNOWN || io_cap == SBT_SSP_IO_CAP_NO_INPUT_NO_OUTPUT || io_cap == SBT_SSP_IO_CAP_DISPLAY_ONLY) {
        mitm_req = 0;
    }
    gap_ssp_set_authentication_requirement((uint16_t)trust * 2 + mitm_req);
    user_io_cap = io_cap;
    user_ssp_request_callback = req_cb;
    user_ssp_result_callback = result_cb;
    sbt_pairing_register_packet_handle();
    return ESP_OK;
}

typedef struct {
    bd_addr_t mac;
    bool accept;
    StaticSemaphore_t binSemaphore;
    SemaphoreHandle_t binSemaphoreHandler;
} confirm_pincode_argv;

static void execute_on_btstack_confirm_pincode(void *data) {
    confirm_pincode_argv *argv = (confirm_pincode_argv*) data;
    if (argv->accept) {
        gap_ssp_confirmation_response(argv->mac);
    }else {
        gap_ssp_confirmation_negative(argv->mac);
    }
    xSemaphoreGive(argv->binSemaphoreHandler);
}

esp_err_t sbt_pairing_comfirm_pincode(bd_addr_t mac, bool accept) {
    confirm_pincode_argv argv;
    memcpy(argv.mac, mac, 6);
    argv.accept = accept;
    argv.binSemaphoreHandler = xSemaphoreCreateBinaryStatic(&argv.binSemaphore);
    btstack_context_callback_registration_t cb_reg;
    cb_reg.callback = &execute_on_btstack_confirm_pincode;
    cb_reg.context = (void*) &argv;
    btstack_run_loop_execute_on_main_thread(&cb_reg);
    xSemaphoreTake(argv.binSemaphoreHandler, portMAX_DELAY);
    return ESP_OK;
}

typedef struct {
    bd_addr_t mac;
    uint32_t pincode;
    StaticSemaphore_t binSemaphore;
    SemaphoreHandle_t binSemaphoreHandler;
} provide_pincode_argv;

static void execute_on_btstack_provide_pincode(void *data) {
    provide_pincode_argv *argv = (provide_pincode_argv*) data;
    if (argv->pincode > 999999) {
        gap_ssp_passkey_negative(argv->mac);
    }else {
        gap_ssp_passkey_response(argv->mac, argv->pincode);
    }
    xSemaphoreGive(argv->binSemaphoreHandler);
}

esp_err_t sbt_pairing_provide_pincode(bd_addr_t mac, uint32_t pincode){
    provide_pincode_argv argv;
    memcpy(argv.mac, mac, 6);
    argv.pincode = pincode;
    argv.binSemaphoreHandler = xSemaphoreCreateBinaryStatic(&argv.binSemaphore);
    btstack_context_callback_registration_t cb_reg;
    cb_reg.callback = &execute_on_btstack_provide_pincode;
    cb_reg.context = (void*) &argv;
    btstack_run_loop_execute_on_main_thread(&cb_reg);
    xSemaphoreTake(argv.binSemaphoreHandler, portMAX_DELAY);
    return ESP_OK;
}
