#include <stdio.h>
#include <inttypes.h>
#include <esp_netif.h>
#include <esp_event.h>

#include "btstack_config.h"
#include "bnep_lwip.h"
#include "btstack.h"

#include "esp_eth_phy_btstack_bnep.h"

// network types
#define NETWORK_TYPE_IPv4       0x0800
#define NETWORK_TYPE_ARP        0x0806
#define NETWORK_TYPE_IPv6       0x86DD

static bd_addr_t remote_addr;

static int record_id = -1;

static uint16_t bnep_version        = 0;
static uint32_t bnep_remote_uuid    = 0;
static uint16_t bnep_l2cap_psm      = 0;

static uint16_t sdp_bnep_l2cap_psm      = 0;
static uint16_t sdp_bnep_version        = 0;
static uint32_t sdp_bnep_remote_uuid    = 0;

static uint8_t   attribute_value[1000];
static const unsigned int attribute_value_buffer_size = sizeof(attribute_value);

// --- PANU CLI

static uint8_t panu_sdp_record[220];

static btstack_packet_callback_registration_t hci_event_callback_registration;

static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
static void handle_sdp_client_query_result(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
static void handle_sdp_client_record_complete(void);
static void get_string_from_data_element(uint8_t * element, uint16_t buffer_size, char * buffer_data);

static void network_init();

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

/* @section SDP parser callback
 *
 * @text The SDP parsers retrieves the BNEP PAN UUID as explained in
 * Section [on SDP BNEP Query example](#sec:sdpbnepqueryExample}.
 */
static void handle_sdp_client_record_complete(void){
    printf("SDP BNEP Record complete\n");
    // accept first entry or if we foudn a NAP and only have a PANU yet
    if ((bnep_remote_uuid == 0) || (sdp_bnep_remote_uuid == BLUETOOTH_SERVICE_CLASS_NAP && bnep_remote_uuid == BLUETOOTH_SERVICE_CLASS_PANU)){
        bnep_l2cap_psm   = sdp_bnep_l2cap_psm;
        bnep_remote_uuid = sdp_bnep_remote_uuid;
        bnep_version     = sdp_bnep_version;
    }
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
                handle_sdp_client_record_complete();
                // next record started
                record_id = sdp_event_query_attribute_byte_get_record_id(packet);
                printf("SDP Record: Nr: %d\n", record_id);
            }

            if (sdp_event_query_attribute_byte_get_attribute_length(packet) <= attribute_value_buffer_size) {
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
                                    case BLUETOOTH_SERVICE_CLASS_NAP:
                                    case BLUETOOTH_SERVICE_CLASS_GN:
                                        printf("SDP Attribute 0x%04x: BNEP PAN protocol UUID: %04x\n", sdp_event_query_attribute_byte_get_attribute_id(packet), uuid);
                                        sdp_bnep_remote_uuid = uuid;
                                        break;
                                    default:
                                        break;
                                }
                            }
                            break;
                        case 0x0100:    // name
                        case 0x0101:    // protocol
                            get_string_from_data_element(attribute_value, sizeof(str), str);
                            printf("SDP Attribute: 0x%04x: %s\n", sdp_event_query_attribute_byte_get_attribute_id(packet), str);
                            break;
                        case BLUETOOTH_ATTRIBUTE_PROTOCOL_DESCRIPTOR_LIST: {
                            printf("SDP Attribute: 0x%04x\n", sdp_event_query_attribute_byte_get_attribute_id(packet));

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
                                        de_element_get_uint16(des_iterator_get_element(&prot_it), &sdp_bnep_l2cap_psm);
                                        break;
                                    case BLUETOOTH_PROTOCOL_BNEP:
                                        if (!des_iterator_has_more(&prot_it)) continue;
                                        de_element_get_uint16(des_iterator_get_element(&prot_it), &sdp_bnep_version);
                                        break;
                                    default:
                                        break;
                                }
                            }
                            printf("Summary: uuid 0x%04x, l2cap_psm 0x%04x, bnep_version 0x%04x\n", sdp_bnep_remote_uuid, sdp_bnep_l2cap_psm, sdp_bnep_version);

                        }
                            break;
                        default:
                            break;
                    }
                }
            } else {
                fprintf(stderr, "SDP attribute value buffer size exceeded: available %d, required %d\n", attribute_value_buffer_size, sdp_event_query_attribute_byte_get_attribute_length(packet));
            }
            break;

        case SDP_EVENT_QUERY_COMPLETE:
            handle_sdp_client_record_complete();
            fprintf(stderr, "General query done with status %d, bnep psm %04x.\n", sdp_event_query_complete_get_status(packet), bnep_l2cap_psm);
            if (bnep_l2cap_psm){
                /* Create BNEP connection */
                bnep_connect(packet_handler, remote_addr, bnep_l2cap_psm, BLUETOOTH_SERVICE_CLASS_PANU, bnep_remote_uuid);
            } else {
                fprintf(stderr, "No BNEP service found\n");
            }

            break;
    }
}

/* LISTING_START(packetHandler): Packet Handler */
static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    /* LISTING_PAUSE */
    UNUSED(channel);
    UNUSED(size);

    bd_addr_t event_addr;
  
    switch (packet_type) {
        case HCI_EVENT_PACKET:
            switch (hci_event_packet_get_type(packet)) {

                case BTSTACK_EVENT_STATE:
                    if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING){
                        printf("Start SDP BNEP query for remote PAN Network Access Point (NAP).\n");
                        sdp_client_query_uuid16(&handle_sdp_client_query_result, remote_addr, BLUETOOTH_SERVICE_CLASS_NAP);
                    }
                    break;

                case HCI_EVENT_PIN_CODE_REQUEST:
                    // inform about pin code request
                    printf("Pin code request - using '0000'\n");
                    hci_event_pin_code_request_get_bd_addr(packet, event_addr);
                    gap_pin_code_response(event_addr, "0000");
                    break;

                case HCI_EVENT_USER_CONFIRMATION_REQUEST:
                    // inform about user confirmation request
                    printf("SSP User Confirmation Auto accept\n");
                    hci_event_user_confirmation_request_get_bd_addr(packet, event_addr);
                    break;

                /* @text BNEP_EVENT_CHANNEL_OPENED is received after a BNEP connection was established or 
                 * or when the connection fails. The status field returns the error code.
                 */  
                case BNEP_EVENT_CHANNEL_OPENED:
                    if (bnep_event_channel_opened_get_status(packet)) {
                        printf("BNEP channel open failed, status %02x\n", bnep_event_channel_opened_get_status(packet));
                    } else {
                        uint16_t bnep_cid    = bnep_event_channel_opened_get_bnep_cid(packet);
                        uint16_t uuid_source = bnep_event_channel_opened_get_source_uuid(packet);
                        uint16_t uuid_dest   = bnep_event_channel_opened_get_destination_uuid(packet);
                        uint16_t mtu         = bnep_event_channel_opened_get_mtu(packet);
                        bnep_event_channel_opened_get_remote_address(packet, event_addr);
                        printf("BNEP connection open succeeded to %s source UUID 0x%04x dest UUID: 0x%04x, max frame size %u\n", bd_addr_to_str(event_addr), uuid_source, uuid_dest, mtu);

                        bd_addr_t local_addr;
                        gap_local_bd_addr(local_addr);

                        // Init BNEP lwIP Adapter
                        //bnep_lwip_init();

                        // Setup PANU Service via BENP lwIP adapter
                        //bnep_lwip_register_service(BLUETOOTH_SERVICE_CLASS_PANU, 1691);

                        // register callback - to print state
                        //bnep_lwip_register_packet_handler(packet_handler);

                        network_init();

                    }
                    break;
                
                /* @text BNEP_EVENT_CHANNEL_CLOSED is received when the connection gets closed.
                 */
                case BNEP_EVENT_CHANNEL_CLOSED:
                    printf("BNEP channel closed\n");
                    break;

                default:
                    break;
            }
            break;

        case BNEP_DATA_PACKET:
            printf("bnep data: size=%u\n", size);
            // Write out the ethernet frame to the network interface

            break;

        default:
            break;
    }
}

static void network_init(){
    ESP_ERROR_CHECK(esp_netif_init());
    // Create default event loop that running in background
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
    esp_netif_t *eth_netif = esp_netif_new(&cfg);
    ESP_ERROR_CHECK(esp_eth_set_default_handlers(eth_netif));
    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&mac_config);
    esp_eth_phy_t *phy = esp_eth_phy_new_btstack_bnep(&phy_config);
    esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
    esp_eth_handle_t eth_handle = NULL;
    ESP_ERROR_CHECK(esp_eth_driver_install(&config, &eth_handle));
    ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));
    /* start Ethernet driver state machine */
    ESP_ERROR_CHECK(esp_eth_start(eth_handle));
}

int btstack_main(int argc, const char * argv[]){

    // host addr to be connected
    const char * remote_addr_string = "30:6a:85:7e:cd:8d";
    // convert string address to library-friendly one
    sscanf_bd_addr(remote_addr_string, remote_addr);

    // setup lwIP, HTTP, DHCP
    //network_setup();

    // Discoverable
    // the discovered name
    gap_set_local_name("PANU 00:00:00:00:00:00");
    gap_discoverable_control(1);
    // Major: Networking Device, Minor: Toy
    gap_set_class_of_device(0x20800);

    // register for HCI events
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // Initialize L2CAP
    l2cap_init();

    // Init SDP Service Discovery Protocol
    sdp_init();

    // Initialize BNEP
    bnep_init();

    //bnep_lwip_init();
    //bnep_lwip_register_service(BLUETOOTH_SERVICE_CLASS_PANU, 1691);

    // PANU Network Access Type
    memset(panu_sdp_record, 0, sizeof(panu_sdp_record));
    uint16_t network_packet_types[] = { NETWORK_TYPE_IPv4, NETWORK_TYPE_ARP, 0};    // 0 as end of list
    pan_create_panu_sdp_record(panu_sdp_record, sdp_create_service_record_handle(), network_packet_types, NULL, NULL, BNEP_SECURITY_NONE);
    sdp_register_service(panu_sdp_record);
    printf("SDP service record size: %u\n", de_get_len((uint8_t*) panu_sdp_record));

    // Turn on the device 
    hci_power_control(HCI_POWER_ON);
    return 0;
}
