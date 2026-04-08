/*
 * at_util.h — AP Scan AT Commands
 */

#ifndef AT_UTIL_H
#define AT_UTIL_H

void copy_mac_byte_to_cstr(char *dst, uint8_t *src, uint16_t offset);
void copy_mac_byte_to_byte(uint8_t *dst, const uint8_t *src, uint16_t offset);
void copy_mac_cstr_to_byte(uint8_t *dst, char *src);

void debug_dump_mac(const char *TAG, uint8_t *src, char *text);

#endif /* AT_UTIL_H */
