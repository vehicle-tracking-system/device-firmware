# Tracker for vehicle tracking system

This repository is part of project for tracking vehicles. Main repository with backend server can be found [here](https://gitlab.fit.cvut.cz/jehlima2/vehicle-tracking-system).

## Configuration
Whole configuration is located in `Constants.h` header file. Default configuration should look like:

```cpp
const int MPU_ADDR = 0x68;

const int POSITION_FREQUENCE = 4000; // ms
const int REPORT_FREQUENCE = 5 * POSITION_FREQUENCE; // ms

static char MQTT_HOST[] = { REPLACE ME };
static int MQTT_PORT = { REPLACE ME };
static char MQTT_TOPIC[] = { REPLACE ME };

const char APN_HOST[] = { REPLACE ME };
const char APN_USER[] = { REPLACE ME };
const char APN_PASS[] = { REPLACE ME };

static char WIFI_SSID[] = "tracker";
static char WIFI_PASSWORD[] = "12345678";

static char STATIC_FILES_PREFIX[] = "/w";
static int HTTP_PORT = 80;
static int DNS_PORT = 53;
static char WEBSOCKET_PATH[] = "/ws";

static float MINIMAL_ACCURACY = 3.0;
static float SPEED_LOWER_LIMIT = 3.0;
static float SPEED_UPPER_LIMIT = 200.0;
```

Variable name | Description
--- | --- 
MPU_ADDR | I2C address of accelerometer
POSITION_FREQUENCE | sampling frequency of actual vehicle position
REPORT_FREQUENCE | frequency of sending reports to the MQTT broker
MQTT_HOST | address of the MQTT broker
MQTT_PORT | port of the MQTT broker
MQTT_TOPIC | topic where all reports will be send
APN_HOST | APN name of your mobile service provider
APN_USER | APN username (usually empty) 
APN_PASS | APN password (usually empty) 
WIFI_SSID | name of the wifi network broadcast by the tracker
WIFI_PASSWORD | password to the wifi network broadcast by the tracker
STATIC_FILES_PREFIX | path to frontend in SPIFFS (other) filesystem
HTTP_PORT | number of port where the web ui will be exposed 
WEBSOCKET_PATH | path to endpoint where it will be possible to connect
MINIMAL_ACCURACY | data from GPS with lower accuracy then this threshold will be ignored
SPEED_LOWER_LIMIT | data from GPS with lower speed then this threshold will be ignored
SPEED_UPPER_LIMIT | data from GPS with higher speed then this threshold will be ignored


Variables above allows to modify just technical aspects of tracker. The configuration and pairing with backend is done by the UI. This is the reason why tracker needs to configure WiFi SSID and password. Connect to WiFi broadcast by tracker (`WIFI_SSID`). Once you will be connected, you can type `tracker.local` into address line in your browser. User interface will appear and you'll be prompted to upload configuration file. You can generate a configuration file in the web administration (more info here: *TO DO*). 
## Build
For proper functioning `PlatformIO` framework is required!

There is `upload-all.sh` build script that compile whole code and uploads it to ESP (ports can be changed in `platformio.ini`)