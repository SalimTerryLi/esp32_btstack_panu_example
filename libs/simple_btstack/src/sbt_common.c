#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "btstack_defines.h"
#include "btstack_run_loop.h"
#include "simple_btstack.h"
#include "sbt_common.h"

static const char *TAG = "SBT_WRAP";

sbt_packet_handler_t sbt_packet_handler = {
    .handlers[0 ... sbt_packet_handler_size - 1] = NULL,
    .mutex_lock = NULL,
};
StaticSemaphore_t sbt_packet_handler_mutex_lock_static_sem;

TaskHandle_t btstack_task_handler = NULL;
sbt_state_t sbt_state = BTSTACK_STOPPED;
run_on_btstack_context sbt_init_stage_call_context;

static void exec_on_btstack_cb(void *data) {
    run_on_btstack_context *context = data;
    context->ret = context->fn(context->data);
    xSemaphoreGive(context->sem);
}

esp_err_t exec_on_btstack(esp_err_t (*fn)(void *data), void* data) {
    // shared StaticSemaphore_t
    StaticSemaphore_t binSemaphore;
    if (sbt_state == BTSTACK_STOPPED) {
        ESP_LOGE(TAG, "SBT not running!");
        return ESP_ERR_INVALID_STATE;
    }
    if (sbt_state == BTSTACK_INIT) {
        sbt_init_stage_call_context.sem = xSemaphoreCreateBinaryStatic(&binSemaphore);
        sbt_init_stage_call_context.ret = ESP_OK;
        sbt_init_stage_call_context.fn = fn;
        sbt_init_stage_call_context.data = data;
        xSemaphoreGive(bin_sem_notify_init_func);
        xSemaphoreTake(sbt_init_stage_call_context.sem, portMAX_DELAY);
        return sbt_init_stage_call_context.ret;
    }
    if (sbt_state == BTSTACK_RUNNING) {
        run_on_btstack_context context = {
            .data = data,
            .fn = fn,
            .ret = ESP_OK,
            .sem = xSemaphoreCreateBinaryStatic(&binSemaphore),
        };
        btstack_context_callback_registration_t cb_reg = {
            .callback = &exec_on_btstack_cb,
            .context = (void *) &context,
        };
        btstack_run_loop_execute_on_main_thread(&cb_reg);
        xSemaphoreTake(context.sem, portMAX_DELAY);
        return context.ret;
    }
    return ESP_ERR_INVALID_STATE;
}