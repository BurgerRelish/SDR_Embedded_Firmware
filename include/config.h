#ifndef CONFIG_H
#define CONFIG_H

/**
 * ==============================================
 * |                                            |
 * |       Control Unit Device Parameters       |
 * |                                            |
 * ==============================================
 */

#define UNIT_UUID "h1u212"
#define VERSION "0001/0001"

/**
 * ==============================================
 * |                                            |
 * |             WiFi Configuration             |
 * |                                            |
 * ==============================================
 */

#define WIFI_SETUP_HOSTNAME "SmartDemand Unit"
#define WIFI_SETUP_AP_PASSWORD "Password123"

/**
 * ==============================================
 * |                                            |
 * |          AsyncElegantOTA Security          |
 * |                                            |
 * ==============================================
 */

#define OTA_USERNAME "sdr"
#define OTA_PASSWORD "updateMe1"

/**
 * ==============================================
 * |                                            |
 * |     Network Time Protocol Configuration    |
 * |                                            |
 * ==============================================
 */

#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET 7200

#endif