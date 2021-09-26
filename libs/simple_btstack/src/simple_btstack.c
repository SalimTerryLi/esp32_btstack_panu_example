#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "simple_btstack.h"
#include "sbt_common.h"
#include "btstack.h"
#include "btstack_port_esp32.h"
#include "gap.h"
#include "esp_log.h"
#include "esp_check.h"

static const char* TAG = "SBT";

typedef struct {
    const char* name;
    uint32_t cod;
    esp_err_t ret_val;
} sbt_conf_t;

/*
 * HCI callback registration struct. should always be accessible during BTStack running
 */
static btstack_packet_callback_registration_t hci_event_callback_registration;

/*
 * Binary semaphore for init stage notification (SBT -> BTStack)
 */
static StaticSemaphore_t bin_sem_buffer_notify_init_func;
SemaphoreHandle_t bin_sem_notify_init_func = NULL;
/*
 * Binary semaphore for init stage notification (BTStack -> SBT)
 */
static StaticSemaphore_t bin_sem_buffer_notify_init_ret;
SemaphoreHandle_t bin_sem_notify_init_ret = NULL;

static sbt_conf_t init_stage_context = {};

/*
 * Binary semaphore for HCI_EVENT_CHANGE and HCI statue notification
 */
static StaticSemaphore_t bin_sem_buffer_notify_hci_state_change;
static SemaphoreHandle_t bin_sem_notify_hci_state_change = NULL;
static volatile HCI_STATE currentHCIState = HCI_STATE_OFF;

static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(channel);
    UNUSED(size);

    //bd_addr_t event_addr;

    switch (packet_type) {
        case HCI_EVENT_PACKET:
            switch (hci_event_packet_get_type(packet)) {
                case BTSTACK_EVENT_STATE:
                    ESP_LOGD(TAG, "HCI state changed from %d to %d", currentHCIState, btstack_event_state_get_state(packet));
                    currentHCIState = btstack_event_state_get_state(packet);
                    xSemaphoreGive(bin_sem_notify_hci_state_change);
                    break;
                default:
                    break;
            }
            break;

        default:
            break;
    }

    for (int i = 0; i < sbt_packet_handler_size; ++i){
        xSemaphoreTake(sbt_packet_handler.mutex_lock, portMAX_DELAY);
        if (sbt_packet_handler.handlers[i] != NULL){
            sbt_packet_handler.handlers[i](packet_type, channel, packet, size);
        }
        xSemaphoreGive(sbt_packet_handler.mutex_lock);
    }
}

static void run_btstack_main_task(void* arg){
    ESP_LOGD(TAG, "BTStack task started");
    if (btstack_init() != 0) {
        ESP_LOGE(TAG, "Failed to init BTStack");
        init_stage_context.ret_val = ESP_ERR_INVALID_STATE;
        xSemaphoreGive(bin_sem_notify_init_ret);
        goto task_exit;
    }
    gap_set_local_name(init_stage_context.name);
    gap_set_class_of_device(init_stage_context.cod);
    // by default disable SSP, which is enabled in hci_init()
    gap_ssp_set_enable(0);
    gap_ssp_set_auto_accept(0);

    // main packet callback
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    l2cap_init();

    sdp_init();

    ESP_LOGD(TAG, "BTStack init finished");
    // init finished, notify parent task
    xSemaphoreGive(bin_sem_notify_init_ret);
    // block for future configuration
    while (true){
        xSemaphoreTake(bin_sem_notify_init_func, portMAX_DELAY);
        if (sbt_init_stage_call_context.fn == NULL) {
            // init finished
            break;
        }
        // else continue init
        sbt_init_stage_call_context.ret = sbt_init_stage_call_context.fn(sbt_init_stage_call_context.data);
        xSemaphoreGive(sbt_init_stage_call_context.sem);
    }

    // start HCI
    if (hci_power_control(HCI_POWER_ON) != 0){
        init_stage_context.ret_val = ESP_ERR_INVALID_STATE;
        xSemaphoreGive(bin_sem_notify_init_ret);
        goto task_exit;
    } else {
        init_stage_context.ret_val = ESP_OK;
        xSemaphoreGive(bin_sem_notify_init_ret);
    }

    ESP_LOGD(TAG, "BTStack main loop started");
    sbt_state = BTSTACK_RUNNING;
    btstack_run_loop_execute();
    sbt_state = BTSTACK_INIT;

task_exit:
    vTaskDelete(btstack_task_handler);
    btstack_task_handler = NULL;
    ESP_LOGD(TAG, "BTStack task exited");
}

esp_err_t sbt_init(const char* name, uint32_t class_of_device){
    sbt_packet_handler.mutex_lock = xSemaphoreCreateMutexStatic(&sbt_packet_handler_mutex_lock_static_sem);

    bin_sem_notify_init_func = xSemaphoreCreateBinaryStatic(&bin_sem_buffer_notify_init_func);

    bin_sem_notify_hci_state_change = xSemaphoreCreateBinaryStatic(&bin_sem_buffer_notify_hci_state_change);

    init_stage_context.name = name;
    init_stage_context.cod = class_of_device;

    bin_sem_notify_init_ret = xSemaphoreCreateBinaryStatic(&bin_sem_buffer_notify_init_ret);
    xTaskCreate(run_btstack_main_task, "btstack_loop", 4096, NULL, uxTaskPriorityGet(xTaskGetCurrentTaskHandle()) + 1, &btstack_task_handler);
    // wait for BTStack init
    xSemaphoreTake(bin_sem_notify_init_ret, portMAX_DELAY);
    sbt_state = BTSTACK_INIT;

    return init_stage_context.ret_val;
}

static esp_err_t sbt_set_discoverable_on_btstack(void* data){
    gap_discoverable_control(*(bool*)data);
    return ESP_OK;
}

esp_err_t sbt_set_discoverable(bool val){
    return exec_on_btstack(sbt_set_discoverable_on_btstack, &val);
}

static esp_err_t sbt_set_connectable_on_btstack(void* data) {
    gap_connectable_control(*(bool*)data);
    return ESP_OK;
}

esp_err_t sbt_set_connectable(bool val){
    return exec_on_btstack(sbt_set_connectable_on_btstack, &val);
}

esp_err_t sbt_start(){
    ESP_RETURN_ON_FALSE(btstack_task_handler != NULL, ESP_ERR_INVALID_STATE, TAG, "sbt not initialized");
    sbt_init_stage_call_context.fn = NULL;
    xSemaphoreGive(bin_sem_notify_init_func);

    // wait HCI power set
    xSemaphoreTake(bin_sem_notify_init_ret, portMAX_DELAY);
    ESP_RETURN_ON_ERROR(init_stage_context.ret_val, TAG, "Failed to set HCI power");

    // wait for HCI is up
    while (currentHCIState != HCI_STATE_WORKING){
        xSemaphoreTake(bin_sem_notify_hci_state_change, portMAX_DELAY);
    }
    return init_stage_context.ret_val;
}