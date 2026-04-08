/*
 * at_custom_deauth_cmd.h — Deauth AT Commands
 */

#ifndef AT_CUSTOM_DEAUTH_CMD_H
#define AT_CUSTOM_DEAUTH_CMD_H

#include <stdbool.h>

/**
 * Register Deauth AT commands:
 *   AT+DEAUTH=1,<ch>,<mac>,<bssid>  Start deauth attack on channel
 *   AT+DEAUTH=0                     Stop deauth
 *   AT+DEAUTH?                      Query status
 *
 * Deauth frames output as:
 *   +DEAUTH:<frames_sent>
 */
bool esp_at_custom_deauth_cmd_register(void);

#endif /* AT_CUSTOM_DEAUTH_CMD_H */
