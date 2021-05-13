#ifndef TRACKER_CONSTANTS_H
#define TRACKER_CONSTANTS_H

#include <FS.h>
#include <SPIFFS.h>

#include "../lib/Tasker/Tasker.h"

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

#define AP_AUTO_OFF
#define USE_ACCELEROMETER // IF DEFINED, TRACKER RECOGNIZE MOVEMENT USING ACCELEROMETER
//#define WIFI_MODE // IF DEFINED, TRACKER CONNECT YOURSELF TO WIFI INSTEAD CREATING AP

static const char WIFI_MODE_SSID[] = "";
static const char WIFI_MODE_PASS[] = "";

static const int MPU_ADDR = 0x69;
static const int MOVEMENT_THRESHOLD = 3000;

static const int POWER_PIN = 5;

static const int RED_LED_PIN = 26;
static const int GREEN_LED_PIN = 25;

static const int POSITION_FREQUENCE = 3000; // ms
static const int POSITIONS_IN_REPORT = 5;
static const int REPORT_FREQUENCE = POSITIONS_IN_REPORT * POSITION_FREQUENCE; // ms

static const double MAX_DISTANCE_BETWEEN_TWO_POSITIONS = round(200 / 3.6) * POSITIONS_IN_REPORT;

static const char WIFI_SSID[] = "tracker";
static const char WIFI_PASSWORD[] = "12345678";

static const char STATIC_FILES_PREFIX[] = "/w";
static const int HTTP_PORT = 80;
static const int DNS_PORT = 53;
static const char WEBSOCKET_PATH[] = "/ws";

static const float MINIMAL_ACCURACY = 3.0;
static const float SPEED_LOWER_LIMIT = 5.0;
static const float SPEED_UPPER_LIMIT = 200.0;

#define FileSystem SPIFFS

#endif //TRACKER_CONSTANTS_H