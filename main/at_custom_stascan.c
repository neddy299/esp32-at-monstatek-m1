#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "esp_at.h"
#include "esp_log.h"
#include "at_util.h"

static const char *TAG = "stascan";

const wifi_promiscuous_filter_t wifi_filt = {.filter_mask=WIFI_PROMIS_FILTER_MASK_DATA};
// const wifi_promiscuous_filter_t wifi_filt = {.filter_mask=WIFI_PROMIS_FILTER_MASK_MGMT | WIFI_PROMIS_FILTER_MASK_DATA};

static bool                 s_stascan_active = false;
static uint8_t              s_channel = 0;

static uint8_t s_target_bssid[6] = { 0 };

//TODO replace with a real linked list lib
typedef struct station {
    uint8_t mac[6];
    struct station *next;
} station_t;
static station_t *s_station_head = NULL;
static station_t *s_station_tail = NULL;


void ap_sniffer_callback(void *buf, wifi_promiscuous_pkt_type_t type) {
    wifi_promiscuous_pkt_t *snifferPacket = (wifi_promiscuous_pkt_t*)buf;
    wifi_pkt_rx_ctrl_t ctrl = (wifi_pkt_rx_ctrl_t)snifferPacket->rx_ctrl;

    if (type != WIFI_PKT_DATA) {
        return;
    }

    // Check if frame has ap in list of APs and determine position
    uint8_t frame_offset = 0;
    int offsets[2] = {10, 4};
    bool matched_ap = false;
    bool ap_is_src = false;
    bool mac_match = true;

    // Check both address offsets for each AP addr
    for (int y = 0; y < 2; y++) {
        // NOTE: We are only processing a single access point, specified by s_target_bssid
        mac_match = true;

        // Go through each byte in addr
        for (int x = 0; x < 6; x++) {
            if (snifferPacket->payload[x + offsets[y]] != s_target_bssid[x]) {
                mac_match = false;
                break;
            }
        }
        if (mac_match) {
            matched_ap = true;
            if (offsets[y] == 10)
                ap_is_src = true;
            break;
        }
        if (matched_ap)
            break;
    }

    // If did not find ap from list in frame, drop frame
    if (!matched_ap) {
        return;
    } else {
        if (ap_is_src)
            frame_offset = 4;
        else
            frame_offset = 10;
    }

    // Check if the station is in our list
    bool in_list = false;
    station_t *station = s_station_head;
    while (station != NULL)
    {
        mac_match = true;
        for (int x = 0; x < 6; x++) {
            if (snifferPacket->payload[x + frame_offset] != station->mac[x]) {
                mac_match = false;
                break;
            }
        }
        if (mac_match) {
            in_list = true;
            break;
        }
        station = station->next;
    }

    // Abort if MAC is already known or dest is broadcast
    char dst_addr[] = "00:00:00:00:00:00";
    copy_mac_byte_to_cstr(dst_addr, snifferPacket->payload, 4);
    if ((in_list) || (strcmp(dst_addr, "ff:ff:ff:ff:ff:ff") == 0)) {
      return;
    }

    // Debug info
    char sta_addr[] = "00:00:00:00:00:00";
    copy_mac_byte_to_cstr(sta_addr, snifferPacket->payload, frame_offset);
    ESP_LOGD(TAG, "matched_ap: %d mac_match: %d ap_is_src: %d frame_offset: %d dst_addr: %s src_addr: %s", matched_ap, mac_match, ap_is_src, frame_offset, dst_addr, sta_addr);

    station_t *new_station = (station_t *)malloc(sizeof(station_t));
    if (new_station == NULL) {
        ESP_LOGE(TAG, "new station malloc failed");
        return;
    }

    // Add station to the list
    if (s_station_tail != NULL) {
        s_station_tail->next = new_station;
    } else {
        s_station_head = new_station;
    }
    s_station_tail = new_station;

    copy_mac_byte_to_byte(new_station->mac, snifferPacket->payload, frame_offset);
    new_station->next = NULL;

    ESP_LOGI(TAG, "NEW station found: %s", sta_addr);

    static uint8_t buffer[64] = {0};
    snprintf((char *)buffer, 64, "+STASCAN:(\"%s\")\r\n", sta_addr);
    esp_at_port_write_data(buffer, strlen((char *)buffer));
}

/* AT+STASCAN? — query status */
static uint8_t at_query_cmd_stascan(uint8_t *cmd_name)
{
    static uint8_t buffer[64] = {0};
    snprintf((char *)buffer, 64, "+STASCAN:(%d,%u)\r\n", s_stascan_active ? 1 : 0, s_channel);
    esp_at_port_write_data(buffer, strlen((char *)buffer));

    return ESP_AT_RESULT_CODE_OK;
}

/* AT+STASCAN=<enable>[,<channel>,<bssid>]  */
static uint8_t at_setup_cmd_stascan(uint8_t para_num)
{
    int32_t enable = 0;
    if (esp_at_get_para_as_digit(0, &enable) != ESP_AT_PARA_PARSE_RESULT_OK) {
        return ESP_AT_RESULT_CODE_ERROR;
    }

    if (enable == 0) {
        /* === STOP stascan === */
        if (!s_stascan_active) {
            return ESP_AT_RESULT_CODE_OK;  /* already stopped */
        }
        s_stascan_active = false;

        esp_wifi_set_promiscuous(false);
        s_channel = 0;

        ESP_LOGI(TAG, "stascan stopped");
        return ESP_AT_RESULT_CODE_OK;
    } else {
        if (para_num != 3) {
            ESP_LOGI(TAG, "invalid parameter length, got %d parameters, wanted 1 or 3", para_num);
            return ESP_AT_RESULT_CODE_ERROR;
        }

        int32_t channel = 0;
        if (esp_at_get_para_as_digit(1, &channel) != ESP_AT_PARA_PARSE_RESULT_OK) {
            return ESP_AT_RESULT_CODE_ERROR;
        }

        // read target BSSID then convert to bytes
        uint8_t *bssid = NULL;
        if (esp_at_get_para_as_str(2, &bssid) != ESP_AT_PARA_PARSE_RESULT_OK) {
            return ESP_AT_RESULT_CODE_ERROR;
        }

        copy_mac_cstr_to_byte(s_target_bssid, (char*)bssid);
        debug_dump_mac(TAG, s_target_bssid, "target BSSID");

         if (s_stascan_active) {
            ESP_LOGI(TAG, "stascan already started");
            return ESP_AT_RESULT_CODE_OK;
        }

        // Free allocated memory
        station_t *station = s_station_head;
        while (station != NULL)
        {
            station_t *next = station->next;
            free(station);
            station = next;
        }
        s_station_head = NULL;
        s_station_tail = NULL;

        // Unusued Maurader32 bits
        // esp_netif_init();
        // esp_event_loop_create_default();

        // esp_wifi_init(&cfg2);
        // #ifdef HAS_IDF_3
        //     esp_wifi_set_country(&country);
        //     esp_event_loop_create_default();
        // #endif

        s_channel = channel;
        esp_wifi_set_mode(WIFI_MODE_NULL);
        esp_wifi_set_channel(s_channel, WIFI_SECOND_CHAN_NONE);

        esp_wifi_set_storage(WIFI_STORAGE_RAM);
        esp_wifi_start();
        //this->setMac();       // more unusued Maurader32 bits
        esp_wifi_set_promiscuous(true);
        esp_wifi_set_promiscuous_filter(&wifi_filt);
        esp_wifi_set_promiscuous_rx_cb(&ap_sniffer_callback);

        s_stascan_active = true;
        ESP_LOGI(TAG, "stascan started, channel: %d", s_channel);
    }

    return ESP_AT_RESULT_CODE_OK;
}

static const esp_at_cmd_struct s_stascan_cmd_list[] = {
    {"+STASCAN", NULL, at_query_cmd_stascan, at_setup_cmd_stascan, NULL},
};

bool esp_at_custom_stascan_cmd_register(void)
{
    ESP_LOGI(TAG, "esp_at_custom_stascan_register");
    return esp_at_custom_cmd_array_regist(s_stascan_cmd_list, sizeof(s_stascan_cmd_list) / sizeof(s_stascan_cmd_list[0]));
}
