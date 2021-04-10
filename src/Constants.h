#ifndef TRACKER_CONSTANTS_H
#define TRACKER_CONSTANTS_H

#include <FS.h>
#include <SPIFFS.h>

#include "Tasker.h"

//
// MESSAGE LEVELS
//
#define DEBUG
#define ERROR
////

#define TINY_GSM_MODEM_SIM808
//#define TINY_GSM_DEBUG Serial // UNCOMMENT THIS FOR SHOW DEBUG LOGS IN SERIAL
#define SERIALGSM Serial2

#ifdef DEBUG
#define DEBUG_PRINT(x, ...)  Serial.print("[DBG] "); Serial.printf(x, __VA_ARGS__)
#else
#define DEBUG_PRINT(x, ...)
#endif

#ifdef ERROR
#define ERROR_PRINT(x, ...)  Serial.print("[ERR] "); Serial.printf(x, __VA_ARGS__)
#else
#define ERROR_PRINT(x, ...)
#endif

const int MPU_ADDR = 0x68;

const int POSITION_FREQUENCE = 4000; // ms
const int REPORT_FREQUENCE = 5 * POSITION_FREQUENCE; // ms

static char MQTT_HOST[] = "mqtt.dev.jehlicka.eu";
static int MQTT_PORT = 8883;
static char MQTT_TOPIC[] = "sledovac-dev";

const char APN_HOST[] = "internet.t-mobile.cz";
const char APN_USER[] = "";
const char APN_PASS[] = "";

static char WIFI_SSID[] = "tracker";
static char WIFI_PASSWORD[] = "12345678";

static char STATIC_FILES_PREFIX[] = "/w";
static int HTTP_PORT = 80;
static int DNS_PORT = 53;
static char WEBSOCKET_PATH[] = "/ws";

static float MINIMAL_ACCURACY = 3.0;
static float SPEED_LOWER_LIMIT = 3.0;
static float SPEED_UPPER_LIMIT = 200.0;

#define FileSystem SPIFFS

#endif //TRACKER_CONSTANTS_H