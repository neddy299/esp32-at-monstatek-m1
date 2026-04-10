/*
 * at_custom_stascan_cmd.h — Station Scanner AT Commands
 */

#ifndef AT_CUSTOM_STASCAN_CMD_H
#define AT_CUSTOM_STASCAN_CMD_H

#include <stdbool.h>

/**
 * Register Stascan AT commands:
 *   AT+STASCAN=1,<ch>,<bssid>  Start station scanning on channel and access point BSSID
 *   AT+STASCAN=0                     Stop stascan
 *   AT+STASCAN?                      Query status
 *
 * Station scanner output as:
 *   +STASCAN:("<mac>")
 *   mac: MAC address for detected station
 *
 */
bool esp_at_custom_stascan_cmd_register(void);

#endif /* AT_CUSTOM_STASCAN_CMD_H */
