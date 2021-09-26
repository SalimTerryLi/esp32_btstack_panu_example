#include "utils/session.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include <freertos/semphr.h>

static StaticSemaphore_t xMutexBuffer;
static SemaphoreHandle_t xSemaphore = NULL;

void mutex_lock() {
    if (xSemaphore == NULL) {
        xSemaphore = xSemaphoreCreateMutexStatic( &xMutexBuffer );
    }
    xSemaphoreTake(xSemaphore, portMAX_DELAY);
}

void mutex_unlock() {
    xSemaphoreGive(xSemaphore);
}

typedef struct session_s session_t;

struct session_s {
    session_t *next;
    mac_addr mac;
    uint32_t tag;
    uint8_t payload[0];
};

static session_t begin_node = {
    .next = NULL,
};

/*
 * always append to tail
 */
void *create_session(mac_addr addr, uint32_t tag, uint32_t size){
    mutex_lock();
    session_t *lastnode = &begin_node;
    while (lastnode->next != NULL) {
        lastnode = lastnode->next;
        if (memcmp(addr, lastnode->mac, 6) == 0 && lastnode->tag == tag) {
            mutex_unlock();
            return NULL;
        }
    }
    lastnode->next = malloc(size + sizeof(session_t));
    lastnode = lastnode->next;
    lastnode->next = NULL;
    memcpy(lastnode->mac, addr, 6);
    lastnode->tag = tag;
    mutex_unlock();
    return lastnode->payload;
}

int delete_session(mac_addr addr, uint32_t tag){
    mutex_lock();
    session_t *lastnode = &begin_node;
    while (lastnode->next != NULL) {
        // next node exist
        if (memcmp(addr, lastnode->next->mac, 6) == 0 && lastnode->next->tag == tag) {
            // next node match
            session_t *next_next_node = lastnode->next->next;
            free(lastnode->next);
            lastnode->next = next_next_node;
            mutex_unlock();
            return 0;
        } else{
            lastnode = lastnode->next;
        }
    }
    // not found
    mutex_unlock();
    return -1;
}

void *find_session(mac_addr addr, uint32_t tag){
    mutex_lock();
    session_t *lastnode = &begin_node;
    while (lastnode->next != NULL) {
        lastnode = lastnode->next;
        if (memcmp(addr, lastnode->mac, 6) == 0 && lastnode->tag == tag) {
            mutex_unlock();
            return lastnode->payload;
        }
    }
    // not found
    mutex_unlock();
    return NULL;
}