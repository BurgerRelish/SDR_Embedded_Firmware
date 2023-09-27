/**
_____ _                        _    ___ _____  _    
| ____| | ___  __ _  __ _ _ __ | |_ / _ \_   _|/ \   
|  _| | |/ _ \/ _` |/ _` | '_ \| __| | | || | / _ \  
| |___| |  __/ (_| | (_| | | | | |_| |_| || |/ ___ \ 
|_____|_|\___|\__, |\__,_|_| |_|\__|\___/ |_/_/   \_\
              |___/                                  
*/

/**
 * 
 * @name ElegantOTA
 * @author Ayush Sharma (ayush@softt.io)
 * @brief 
 * @version 3.0.0
 * @date 2023-08-30
 */

#ifndef ElegantOTA_h
#define ElegantOTA_h

#include "Arduino.h"
#include "stdlib_noniso.h"
#include "elop.h"

#ifndef ELEGANTOTA_USE_ASYNC_WEBSERVER
  #define ELEGANTOTA_USE_ASYNC_WEBSERVER 1
#endif

#define ELEGANTOTA_DEBUG 1
#define UPDATE_DEBUG 1

#if ELEGANTOTA_DEBUG
  #define ELEGANTOTA_DEBUG_MSG(x) Serial.printf("%s %s", "[ElegantOTA] ", x)
#else
  #define ELEGANTOTA_DEBUG_MSG(x)
#endif

#if defined(ESP8266)
  #include "FS.h"
  #include "LittleFS.h"
  #include "Updater.h"
  #include "StreamString.h"
  #if ELEGANTOTA_USE_ASYNC_WEBSERVER == 1
    #include "ESPAsyncTCP.h"
    #include "ESPAsyncWebServer.h"
    #define ELEGANTOTA_WEBSERVER AsyncWebServer
  #else
    #include "ESP8266WiFi.h"
    #include "WiFiClient.h"
    #include "ESP8266WebServer.h"
    #define ELEGANTOTA_WEBSERVER ESP8266WebServer
  #endif
  #define HARDWARE "ESP8266"
#elif defined(ESP32)
  #include "FS.h"
  #include "Update.h"
  #include "StreamString.h"
  #if ELEGANTOTA_USE_ASYNC_WEBSERVER == 1
    #include "AsyncTCP.h"
    #include "ESPAsyncWebServer.h"
    #define ELEGANTOTA_WEBSERVER AsyncWebServer
  #else
    #include "WiFi.h"
    #include "WiFiClient.h"
    #include "WebServer.h"
    #define ELEGANTOTA_WEBSERVER WebServer
  #endif
  #define HARDWARE "ESP32"
#endif

enum OTA_Mode {
    OTA_MODE_FIRMWARE = 0,
    OTA_MODE_FILESYSTEM = 1
};


class ElegantOTAClass{
  public:
    ElegantOTAClass();

    void begin(ELEGANTOTA_WEBSERVER *server, const char * username = "", const char * password = "");

    void setAuth(const char * username, const char * password);
    void clearAuth();
    void loop();

    void onStart(void callable(void));
    void onProgress(void callable(size_t current, size_t final));
    void onEnd(void callable(bool success));
    
  private:
    ELEGANTOTA_WEBSERVER *_server;

    bool _authenticate;
    char _username[64];
    char _password[64];

    bool _reboot = false;
    unsigned long _reboot_request_millis = 0;

    String _update_error_str = "";
    unsigned long _current_progress_size;

    void (*preUpdateCallback)() = NULL;
    void (*progressUpdateCallback)(size_t current, size_t final) = NULL;
    void (*postUpdateCallback)(bool success) = NULL;
};

extern ElegantOTAClass ElegantOTA;
#endif
