#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"
#include <stdbool.h>
#include "bluetooth.h"

/*
 * This file serves internal APIs and states
 */

/* BTStack task handler */
extern TaskHandle_t btstack_task_handler;
/* sbt task handler */
extern TaskHandle_t sbt_task_handler;
/* running flag */
extern bool isBTStackMainLoopRunning;

/* packet handler callbacks, return the count number that event get processed */
typedef uint16_t(*packet_handler_func_t)(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
typedef struct sbt_packet_handler_s {
    union{
        struct named_handlers{
            packet_handler_func_t debug_handle;
            packet_handler_func_t pairing_handle;
        } by_name;
        packet_handler_func_t handlers[sizeof(struct named_handlers) / sizeof(packet_handler_func_t)];
    };  // critical resource
    SemaphoreHandle_t mutex_lock;
} sbt_packet_handler_t;
extern sbt_packet_handler_t sbt_packet_handler;
#define sbt_packet_handler_size (sizeof(struct named_handlers) / sizeof(packet_handler_func_t))

/*
 * Enter BTStack's main loop
 * will block until successfully joined if mainloop is running
 * or pass to next statement if mainloop is not running
 */
#define ENTER_BTSTACK_LOOP()                                                            \
    /* stack variables shared with another callback. should be valid during executing */\
    StaticSemaphore_t binSemaphore;                                                     \
    SemaphoreHandle_t binSemaphoreHandler = xSemaphoreCreateBinaryStatic(&binSemaphore);\
    btstack_context_callback_registration_t cb_reg = {                                  \
        .callback = &btstack_eventloop_callback,                                        \
        .context = (void*) &binSemaphoreHandler,                                        \
    };                                                                                  \
    if (isBTStackMainLoopRunning && btstack_task_handler == xTaskGetCurrentTaskHandle()) {  \
        ESP_LOGD(TAG, "Request running on BTStack task");                               \
        btstack_run_loop_execute_on_main_thread(&cb_reg);                               \
        /* block here until callback executed */                                        \
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);                                        \
        ESP_LOGD(TAG, "Now synced with BTStack main task, continuing");                 \
    }

/*
 * Exit BTStack's main loop
 * must be used paired with ENTER_BTSTACK_LOOP
 */
#define EXIT_BTSTACK_LOOP()                             \
    if (isBTStackMainLoopRunning && btstack_task_handler != xTaskGetCurrentTaskHandle()) {  \
        ESP_LOGD(TAG, "Finished executing");            \
        xSemaphoreGive(binSemaphoreHandler);            \
    }

/* shared callback for ENTER_BTSTACK_LOOP --- being executed on parent task context, not BTStack's */
void btstack_eventloop_callback(void* context);

#ifdef __cplusplus
}
#endif
