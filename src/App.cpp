#include <App.h>

#ifdef USE_GSM
    #include <HardwareSerial.h>

    #define TINY_GSM_MODEM_SARAR4
    #define SerialAT Serial1

    #include <TinyGsmClient.h>

    TinyGsm modem(SerialAT);
    TinyGsmClient client(modem);
#else
    #include <WiFi.h>
    WiFiClient client;
#endif


void App::begin()
{
    mqtt_client = new MQTTClient(&client); // Start MQTT Client
    mqtt_client -> begin();
    return;
}