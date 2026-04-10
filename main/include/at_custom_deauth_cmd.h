/*
 * at_custom_deauth_cmd.h — Deauth AT Commands
 */

#ifndef AT_CUSTOM_DEAUTH_CMD_H
#define AT_CUSTOM_DEAUTH_CMD_H

#include <stdbool.h>

/**
 * Register Deauth AT commands:
 *   AT+DEAUTH=1,<ch>,<mac>,<bssid>  Start deauth attack on channel, target station MAC and access point BSSID
 *   AT+DEAUTH=0                     Stop deauth
 *   AT+DEAUTH?                      Query status
 *
 * Deauth attack output as:
 *   +DEAUTH:<frames_sent>
 *
 * Deauth query output as:
 *   +DEAUTH:(<enabled>,<channel>,<enable_mode>,<num_modes>)
 *   enabled: 0 = attack stopped, 1 = attack started
 *   channel: wifi channel #
 *   enable_mode: active deauth enable mode from 1 to <num_modes>
 *   num_modes: number of available modes (NUM_ENABLE_MODES)
 *
 */
bool esp_at_custom_deauth_cmd_register(void);

#endif /* AT_CUSTOM_DEAUTH_CMD_H */
