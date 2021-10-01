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

typedef enum {
    BTSTACK_STOPPED,
    BTSTACK_INIT,
    BTSTACK_RUNNING,
} sbt_state_t;

typedef struct {
    esp_err_t (*fn)(void *data);
    void* data;
    SemaphoreHandle_t sem;
    esp_err_t ret;
} run_on_btstack_context;

/* BTStack task handler */
extern TaskHandle_t btstack_task_handler;
/* running flag */
extern sbt_state_t sbt_state;
/* store calls that needed to be performed on btstack task at init stage */
extern run_on_btstack_context sbt_init_stage_call_context;
/* sem to notify init stage execute requests */
extern SemaphoreHandle_t bin_sem_notify_init_func;

/* packet handler callbacks, return the count number that event get processed */
typedef uint16_t(*packet_handler_func_t)(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
typedef volatile struct sbt_packet_handler_s {
    union{
        struct named_handlers{
            packet_handler_func_t debug_handle;
            packet_handler_func_t pairing_handle;
            packet_handler_func_t bnep_handle;
        } by_name;
        packet_handler_func_t handlers[sizeof(struct named_handlers) / sizeof(packet_handler_func_t)];
    };  // critical resource
    SemaphoreHandle_t mutex_lock;
} sbt_packet_handler_t;
extern sbt_packet_handler_t sbt_packet_handler;
/* static space for mutex_lock */
extern StaticSemaphore_t sbt_packet_handler_mutex_lock_static_sem;
#define sbt_packet_handler_size (sizeof(struct named_handlers) / sizeof(packet_handler_func_t))

/*
 * block and wait for the function returns, within btstack task
 */
esp_err_t exec_on_btstack(esp_err_t (*fn)(void *data), void* data);

#ifdef __cplusplus
}
#endif
