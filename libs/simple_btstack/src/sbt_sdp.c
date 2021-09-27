#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "btstack.h"
#include "gap.h"
#include "simple_btstack.h"
#include "sbt_common.h"
#include "sbt_sdp.h"
#include "utils/session.h"

static const char *TAG = "SBT_SDP";

/* statically allocated attribute value buffer size */
#define ATTRIBUTE_VALUE_BUFFER_SIZE 1024

static uint8_t attribute_value[ATTRIBUTE_VALUE_BUFFER_SIZE] = {};
static int record_id = -1;

// output result
static bd_addr_t query_addr;
static query_result_bnep_t query_result;
static volatile bool query_result_valid = false;

// output notify
static StaticSemaphore_t bin_sem_buffer_query_notify;
SemaphoreHandle_t bin_sem_query_notify = NULL;

// PANU client routines helper function
static void get_string_from_data_element(uint8_t * element, uint16_t buffer_size, char * buffer_data){
    de_size_t de_size = de_get_size_type(element);
    int pos     = de_get_header_size(element);
    int len = 0;
    switch (de_size){
        case DE_SIZE_VAR_8:
            len = element[1];
            break;
        case DE_SIZE_VAR_16:
            len = big_endian_read_16(element, 1);
            break;
        default:
            break;
    }
    uint16_t bytes_to_copy = btstack_min(buffer_size-1,len);
    memcpy(buffer_data, &element[pos], bytes_to_copy);
    buffer_data[bytes_to_copy] ='\0';
}

static void handle_sdp_client_query_result(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {

    UNUSED(packet_type);
    UNUSED(channel);
    UNUSED(size);

    des_iterator_t des_list_it;
    des_iterator_t prot_it;
    char str[20];

    switch (hci_event_packet_get_type(packet)){
        case SDP_EVENT_QUERY_ATTRIBUTE_VALUE:
            // Handle new SDP record
            if (sdp_event_query_attribute_byte_get_record_id(packet) != record_id) {
                //handle_sdp_client_record_complete();
                // next record started
                record_id = sdp_event_query_attribute_byte_get_record_id(packet);
                ESP_LOGD(TAG, "SDP Record: Nr: %d", record_id);
            }

            if (sdp_event_query_attribute_byte_get_attribute_length(packet) <= ATTRIBUTE_VALUE_BUFFER_SIZE) {
                // build packet part by part
                attribute_value[sdp_event_query_attribute_byte_get_data_offset(packet)] = sdp_event_query_attribute_byte_get_data(packet);

                // process packet when fully received
                if ((uint16_t)(sdp_event_query_attribute_byte_get_data_offset(packet)+1) == sdp_event_query_attribute_byte_get_attribute_length(packet)) {

                    switch(sdp_event_query_attribute_byte_get_attribute_id(packet)) {
                        case BLUETOOTH_ATTRIBUTE_SERVICE_CLASS_ID_LIST:
                            if (de_get_element_type(attribute_value) != DE_DES) break;
                            for (des_iterator_init(&des_list_it, attribute_value); des_iterator_has_more(&des_list_it); des_iterator_next(&des_list_it)) {
                                uint8_t * element = des_iterator_get_element(&des_list_it);
                                if (de_get_element_type(element) != DE_UUID) continue;
                                uint32_t uuid = de_get_uuid32(element);
                                switch (uuid){
                                    case BLUETOOTH_SERVICE_CLASS_PANU:
                                        ESP_LOGD(TAG, "SDP Attribute Value: BNEP PAN protocol UUID: PANU");
                                        break;
                                    case BLUETOOTH_SERVICE_CLASS_NAP:
                                        ESP_LOGD(TAG, "SDP Attribute Value: BNEP PAN protocol UUID: NAP");
                                        query_result.sdp_bnep_remote_uuid = uuid;
                                        query_result_valid = true;
                                        break;
                                    case BLUETOOTH_SERVICE_CLASS_GN:
                                        ESP_LOGD(TAG, "SDP Attribute Value: BNEP PAN protocol UUID: GN");
                                        break;
                                    default:
                                        break;
                                }
                            }
                            break;
                        case 0x0100:    // name
                            get_string_from_data_element(attribute_value, sizeof(str), str);
                            ESP_LOGD(TAG, "SDP Attribute: name: %s", str);
                            break;
                        case 0x0101:    // protocol
                            get_string_from_data_element(attribute_value, sizeof(str), str);
                            ESP_LOGD(TAG, "SDP Attribute: protocol: %s", str);
                            break;
                        case BLUETOOTH_ATTRIBUTE_PROTOCOL_DESCRIPTOR_LIST: {
                            ESP_LOGD(TAG, "SDP Attribute: PROTOCOL_DESCRIPTOR_LIST");

                            for (des_iterator_init(&des_list_it, attribute_value); des_iterator_has_more(&des_list_it); des_iterator_next(&des_list_it)) {
                                uint8_t       *des_element;
                                uint8_t       *element;
                                uint32_t       uuid;

                                if (des_iterator_get_type(&des_list_it) != DE_DES) continue;

                                des_element = des_iterator_get_element(&des_list_it);
                                des_iterator_init(&prot_it, des_element);
                                element = des_iterator_get_element(&prot_it);

                                if (!element) continue;
                                if (de_get_element_type(element) != DE_UUID) continue;

                                uuid = de_get_uuid32(element);
                                des_iterator_next(&prot_it);
                                switch (uuid){
                                    case BLUETOOTH_PROTOCOL_L2CAP:
                                        if (!des_iterator_has_more(&prot_it)) continue;
                                        de_element_get_uint16(des_iterator_get_element(&prot_it), &query_result.sdp_bnep_l2cap_psm);
                                        break;
                                    case BLUETOOTH_PROTOCOL_BNEP:
                                        if (!des_iterator_has_more(&prot_it)) continue;
                                        de_element_get_uint16(des_iterator_get_element(&prot_it), &query_result.sdp_bnep_version);
                                        break;
                                    default:
                                        break;
                                }
                            }
                            ESP_LOGD(TAG, "Summary: uuid 0x%04x, l2cap_psm 0x%04x, bnep_version 0x%04x", query_result.sdp_bnep_remote_uuid, query_result.sdp_bnep_l2cap_psm, query_result.sdp_bnep_version);

                        }
                            break;
                        default:
                            break;
                    }
                }
            } else {
                ESP_LOGE(TAG, "SDP attribute value buffer size exceeded: available %d, required %d", ATTRIBUTE_VALUE_BUFFER_SIZE, sdp_event_query_attribute_byte_get_attribute_length(packet));
            }
            break;

        case SDP_EVENT_QUERY_COMPLETE:
            xSemaphoreGive(bin_sem_query_notify);
            ESP_LOGD(TAG, "General query done with status %u.", sdp_event_query_complete_get_status(packet));
            break;
    }
}

typedef struct {
    btstack_packet_handler_t callback;
    bd_addr_t remote;
    uint16_t uuid16;
} sbt_sdp_query_u16_on_btstack_t;

static esp_err_t sbt_sdp_query_u16_on_btstack(void* data) {
    sbt_sdp_query_u16_on_btstack_t *argv = data;
    sdp_client_query_uuid16(argv->callback, argv->remote, argv->uuid16);
    return ESP_OK;
}

esp_err_t sbt_sdp_query_nap(bd_addr_t remote, query_result_bnep_t *result) {
    memcpy(query_addr, remote, sizeof(bd_addr_t));
    query_result_valid = false;
    bin_sem_query_notify = xSemaphoreCreateBinaryStatic(&bin_sem_buffer_query_notify);
    sbt_sdp_query_u16_on_btstack_t argv = {
        .uuid16 = BLUETOOTH_SERVICE_CLASS_NAP,
        .callback = handle_sdp_client_query_result,
    };
    memcpy(argv.remote, remote, sizeof(bd_addr_t));
    exec_on_btstack(sbt_sdp_query_u16_on_btstack, &argv);
    xSemaphoreTake(bin_sem_query_notify, portMAX_DELAY);
    if (query_result_valid) {
        memcpy(result, &query_result, sizeof(query_result_bnep_t));
        return ESP_OK;
    }
    return ESP_ERR_NOT_FOUND;
}