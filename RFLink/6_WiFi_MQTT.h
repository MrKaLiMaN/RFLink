// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#ifndef WiFi_MQTT_h
#define WiFi_MQTT_h

#include <Arduino.h>
#include "RFLink.h"

#ifdef ESP32
#include <WiFi.h>
#elif ESP8266
#include <ESP8266WiFi.h>
#endif

#ifdef USE_WIFIMANAGER
    #include "WifiManager.h"
    extern WiFiManager wifiManager;
    void setup_WifiManager();
    void start_WifiManager();
#endif // USE_WIFIMANAGER



#ifdef MQTT_ENABLED
extern char MQTTbuffer[PRINT_BUFFER_SIZE]; // Buffer for MQTT message

#ifndef USE_WIFIMANAGER
void setup_WIFI();
void start_WIFI();
void stop_WIFI();
#endif // USE_WIFIMANAGER
void setup_MQTT();
void reconnect();
void publishMsg();
void checkMQTTloop();

#else // MQTT_ENABLED

#if (defined(ESP32) || defined(ESP8266))
void setup_WIFI_OFF();
#endif

#endif // MQTT_ENABLED

#endif // WiFi_MQTT_h