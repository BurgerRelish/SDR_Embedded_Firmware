#ifndef CONFIG_H
#define CONFIG_H

/**
 * ==============================================
 * |                                            |
 * |       Control Unit Device Parameters       |
 * |                                            |
 * ==============================================
 */

#define UNIT_UUID "1302c1a0-8244-4b18-be52-616a50527aec"
#define VERSION "0001/0001"

#define DEFAULT_SAMPLE_PERIOD 1
#define DEFAULT_SERIALIZATION_PERIOD 60
#define DEFAULT_MODE 0 // Default mode rule engine
#define DEFAULT_KWH_PRICE 0.6746
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