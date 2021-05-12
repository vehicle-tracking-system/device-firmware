#include <Arduino.h>

#include <Preferences.h>
#include "Constants.h"
#include <logic/StateManager.h>
#include <logic/GSM.h>
#include <networking/WebServer.h>
#include <networking/AP.h>
#include <networking/WifiClient.h>
#include <logic/LedController.h>
#include <logic/Accelerometer.h>

Preferences preferences;
StateManager stateManager(preferences);
Accelerometer accelerometer(stateManager);
LedController ledController(stateManager);
GSM gsm(stateManager);
WebServer webServer;
JsonConnector jsonConnector;

long counter = 0;

// return in setup function can stop all tasks run from there!
void setup() {
    Serial.begin(115200);
    if (!FileSystem.begin()) {
        ERROR_PRINT("Could not init FS\n", NULL);
    }
    DEBUG_PRINT("Initializing...\n", NULL);
    DEBUG_PRINT("Max distance between two positions is %f\n", MAX_DISTANCE_BETWEEN_TWO_POSITIONS);
    preferences.begin("tracker", false);
    stateManager.begin();

#ifdef USE_ACCELEROMETER
    if (!accelerometer.begin()) {
        ERROR_PRINT("Accelerometer init failed!", NULL);
        stateManager.setError("Accelerometer init failed!");
    }

    DefaultTasker.loopEvery("acc", 50, [] {
        if (!accelerometer.updateAcceleration()) {
            ERROR_PRINT("Update acceleration failed\n", NULL);
        }
//        Serial.printf("%ld,%f\n", counter, stateManager.getAcc());
        counter++;
    });
#endif

    // start webserver
    NetworkTasker.once("ap-server", [] {
#ifdef WIFI_MODE
        WifiClient::begin(WIFI_MODE_SSID, WIFI_MODE_PASS);
#else
        String ssid = WIFI_SSID + WiFi.macAddress();
        DEBUG_PRINT("SSID: %s\n", ssid.c_str());
        AP::begin(ssid.c_str(), WIFI_PASSWORD);
#endif
        webServer.begin(jsonConnector);
    });

#ifndef WIFI_MODE
    // turn off wifi AP and webserver after timeout
    NetworkTasker.once("ap-off", [] {
#ifdef AP_AUTO_OFF
        DEBUG_PRINT("Registering promise to turn off AP...\n", NULL);
        Tasker::sleep(120000);
        DEBUG_PRINT("Turning off AP...\n", NULL);
        AP::stop();
        webServer.stop();
#endif
    });
#endif


    jsonConnector.subscribe([](const JsonDocument &json) {
        DEBUG_PRINT("Configuration JSON received\n", NULL);
        if (json["type"].as<String>() == "file") {
            preferences.putString("token", json["token"].as<char *>());
            preferences.putLong("vehicleId", json["vehicleId"].as<long>());
            preferences.putString("mqttHost", json["mqttHost"].as<String>());
            preferences.putString("mqttTopic", json["mqttTopic"].as<String>());
            preferences.putInt("mqttPort", json["mqttPort"].as<int>());
            preferences.putString("mqttUsername", json["mqttUsername"].as<String>());
            preferences.putString("mqttPassword", json["mqttPassword"].as<String>());
        }
        if (json["type"].as<String>() == "form") {
            preferences.putString("apnHost", json["apnHost"].as<String>());
            preferences.putString("apnUser", json["apnUser"].as<String>());
            preferences.putString("apnPass", json["apnPass"].as<String>());
        }
        stateManager.setTextState("Configuration updated...");
    });

    NetworkTasker.loopEvery("state", 100, [] {
        if (stateManager.stateToSend()) {
            DynamicJsonDocument doc(2048);
            doc["state"] = stateManager.textState() + " " + stateManager.getAcc();
            doc["new"] = stateManager.stateToSend();
            doc["latitude"] = stateManager.lat();
            doc["longitude"] = stateManager.lng();
            doc["speed"] = stateManager.speed();
            doc["isMoving"] = stateManager.moving();
            jsonConnector.send(doc);
        }
    });

    if (!gsm.begin()) {
        ERROR_PRINT("GSM initialization failed! Check wiring and reset.\n", NULL);
        return;
    }

    DEBUG_PRINT("Starting the app...\n", NULL);

    // read new position to buffer from GPS
    DefaultTasker.loopEvery("get-position", POSITION_FREQUENCE, [] {
        gsm.enqueueNewPosition();
    });

    // send report with positions to MQTT broker
    DefaultTasker.loopEvery("send-report", REPORT_FREQUENCE, [] {
        if (!gsm.sendReport()) {
            ERROR_PRINT("Send report failed!!!\n", NULL);
        }
    });

    DEBUG_PRINT("Startup finished!\n", NULL);

}

void loop() {
    // no op - everything is handled by native tasks through Tasker
}