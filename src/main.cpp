#include <Arduino.h>

#include <Preferences.h>
#include "Constants.h"
#include <logic/StateManager.h>
#include <logic/GSM.h>
#include <networking/WebServer.h>
#include <networking/AP.h>

Preferences preferences;
StateManager stateManager(preferences);
GSM gsm(stateManager);
WebServer webServer;
JsonConnector jsonConnector;

// return in setup function can stop all tasks run from there!
void setup() {
    Serial.begin(115200);
    if (!FileSystem.begin()) {
        ERROR_PRINT("Could not init FS\n", NULL);
    }
    DEBUG_PRINT("Initializing...\n", NULL);
    preferences.begin("tracker", false);
    stateManager.begin();

    // start webserver
    NetworkTasker.once("ap-server", [] {
        AP::begin(WIFI_SSID, WIFI_PASSWORD);
        webServer.begin(jsonConnector);
    });

    // turn off wifi AP and webserver after timeout
    NetworkTasker.once("ap-off", [] {
        DEBUG_PRINT("Registering promise to turn off AP...\n", NULL);
        Tasker::sleep(120000);
        DEBUG_PRINT("Turning off AP...\n", NULL);
        AP::stop();
        webServer.stop();
    });

    jsonConnector.subscribe([](const JsonDocument &json) {
        DEBUG_PRINT("Configuration JSON received\n", NULL);
        preferences.putString("token", json["token"].as<char *>());
        preferences.putLong("vehicleId", json["vehicleId"].as<long>());
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