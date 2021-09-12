#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "simple_btstack.h"
#include "sbt_common.h"

sbt_packet_handler_t sbt_packet_handler = {
    .handlers[0 ... sbt_packet_handler_size - 1] = NULL,
    .mutex_lock = NULL,
};

/* BTStack task handler */
TaskHandle_t btstack_task_handler = NULL;
/* sbt task handler */
TaskHandle_t sbt_task_handler = NULL;

void btstack_eventloop_callback(void* context){
    /* semaphore is on the original task's stack and is valid during whole callback execution */
    SemaphoreHandle_t *handler = (SemaphoreHandle_t*) context;
    /* notify original task to start */
    xTaskNotifyGive(sbt_task_handler);
    /* block and wait until original task finished */
    xSemaphoreTake(*handler, portMAX_DELAY);
};
