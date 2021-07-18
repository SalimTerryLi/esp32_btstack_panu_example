#include "btstack_esp_netif_driver.h"

#include "esp_err.h"
#include "esp_netif.h"

typedef struct my_netif_driver_s {
    esp_netif_driver_base_t base;           /*!< base structure reserved as esp-netif driver */
    driver_impl             *h;             /*!< handle of driver implementation */
} my_netif_driver_t;

esp_err_t btstack_bnep_transmit(esp_netif_t *esp_netif, void *data, size_t len){
    return ESP_OK;
}

void btstack_bnep_free_rx_buffer(void *esp_netif, void *buffer){

}

esp_err_t btstack_bnep_receive(esp_netif_t *esp_netif, void *buffer, size_t len, void *eb){
    return ESP_OK;
}