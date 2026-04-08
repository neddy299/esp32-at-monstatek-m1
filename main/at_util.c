#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "esp_log.h"


void copy_mac_byte_to_cstr(char *dst, uint8_t *src, uint16_t offset) {
  sprintf(dst, "%02x:%02x:%02x:%02x:%02x:%02x", src[offset+0], src[offset+1], src[offset+2], src[offset+3], src[offset+4], src[offset+5]);
}

void copy_mac_byte_to_byte(uint8_t *mac, const uint8_t *src, uint16_t offset) {
  for (int i = 0; i < 6; i++) {
    mac[i] = src[offset + i];
  }
}

void copy_mac_cstr_to_byte(uint8_t *dst, char *src) {
    char *token = strtok(src, ":");
    for (int i = 0; token != NULL && i < 6; i++) {
        dst[i] = (uint8_t)strtol(token, NULL, 16);
        token = strtok(NULL, ":");
    }
}

void debug_dump_mac(const char *TAG, uint8_t *src, char *text) {
    ESP_LOGD(TAG, "%s: %02x:%02x:%02x:%02x:%02x:%02x", text,src[0], src[1], src[2], src[3], src[4], src[5]);
}
