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
 * Binary semaphore for init stage
 */
static StaticSemaphore_t binSemaphoreBTStackInit;
static SemaphoreHandle_t binSemaphoreHandlerBTStackInit = NULL;
static sbt_conf_t init_conf = {};
/*
 * serves init stage to make sure this function pointer is called from BTStack task
 * if is NULL then init stage is finished, or the function get called
 */
static void (*nextTaskEntrance)(void *) = NULL;
static void *nextTaskEntranceContext = NULL;

bool isBTStackMainLoopRunning = false;

/*
 * Binary semaphore for HCI_EVENT_CHANGE and HCI statue
 */
static StaticSemaphore_t binSemaphore_hci_event_change;
static SemaphoreHandle_t binSemaphoreHandler_hci_event_change = NULL;
static HCI_STATE currentHCIState = HCI_STATE_OFF;

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
                    xSemaphoreGive(binSemaphoreHandler_hci_event_change);
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
        init_conf.ret_val = ESP_ERR_INVALID_STATE;
        xTaskNotifyGive(sbt_task_handler);
        goto task_exit;
    }
    gap_set_local_name(init_conf.name);
    gap_set_class_of_device(init_conf.cod);
    // by default disable SSP, which is enabled in hci_init()
    gap_ssp_set_enable(0);
    gap_ssp_set_auto_accept(0);

    // main packet callback
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    l2cap_init();

    ESP_LOGD(TAG, "BTStack init finished");
    // init finished, notify parent task
    xTaskNotifyGive(sbt_task_handler);
    // block for future configuration
    while (true){
        xSemaphoreTake(binSemaphoreHandlerBTStackInit, portMAX_DELAY);
        if (nextTaskEntrance == NULL) {
            // init finished
            break;
        }
        // else continue init
        nextTaskEntrance(nextTaskEntranceContext);
    }

    // start HCI
    if (hci_power_control(HCI_POWER_ON) != 0){
        ESP_LOGE(TAG, "Failed to set HCI power");
        init_conf.ret_val = ESP_ERR_INVALID_STATE;
        xTaskNotifyGive(sbt_task_handler);
        goto task_exit;
    }

    ESP_LOGD(TAG, "BTStack main loop started");
    isBTStackMainLoopRunning = true;
    btstack_run_loop_execute();
    isBTStackMainLoopRunning = false;

task_exit:
    vTaskDelete(btstack_task_handler);
    btstack_task_handler = NULL;
    ESP_LOGD(TAG, "BTStack task exited");
}

esp_err_t sbt_init(const char* name, uint32_t class_of_device){
    static StaticSemaphore_t sbt_packet_handler_mutex_buffer;
    sbt_packet_handler.mutex_lock = xSemaphoreCreateMutexStatic(&sbt_packet_handler_mutex_buffer);
    sbt_task_handler = xTaskGetCurrentTaskHandle();
    binSemaphoreHandlerBTStackInit = xSemaphoreCreateBinaryStatic(&binSemaphoreBTStackInit);
    binSemaphoreHandler_hci_event_change = xSemaphoreCreateBinaryStatic(&binSemaphore_hci_event_change);
    init_conf.name = name,
    init_conf.cod = class_of_device,
    xTaskCreate(run_btstack_main_task, "btstack_loop", 4096, NULL, uxTaskPriorityGet(xTaskGetCurrentTaskHandle()) + 1, &btstack_task_handler);
    // wait for BTStack init
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    return init_conf.ret_val;
}

esp_err_t sbt_set_discoverable(bool val){
    ENTER_BTSTACK_LOOP();
    gap_discoverable_control(val);
    EXIT_BTSTACK_LOOP();
    return ESP_OK;
}

esp_err_t sbt_set_connectable(bool val){
    ENTER_BTSTACK_LOOP();
    gap_connectable_control(val);
    EXIT_BTSTACK_LOOP();
    return ESP_OK;
}

esp_err_t sbt_start(){
    ESP_RETURN_ON_FALSE(btstack_task_handler != NULL, ESP_ERR_INVALID_STATE, TAG, "sbt not initialized");
    nextTaskEntrance = NULL;
    nextTaskEntranceContext = NULL;
    xSemaphoreGive(binSemaphoreHandlerBTStackInit);

    // wait for HCI is up
    while (currentHCIState != HCI_STATE_WORKING){
        xSemaphoreTake(binSemaphoreHandler_hci_event_change, portMAX_DELAY);
    }
    return init_conf.ret_val;
}