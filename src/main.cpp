#include <Arduino.h>

#include "Constants.h"
#include <logic/NetworkingService.h>
#include <networking/WebServer.h>
#include <networking/AP.h>

StateManager stateManager;
NetworkingService mqttService(stateManager);
WebServer webServer;
JsonConnector jsonConnector;

void setup() {
    Serial.begin(115200);

    DEBUG_PRINT("Initializing...\n", NULL);

    mqttService.begin();

    if (!FileSystem.begin()) {
        ERROR_PRINT("Could not init FS\n", NULL);
//        return;
    }

    NetworkTasker.once("ap-server", [] {
        AP::begin(WIFI_SSID, WIFI_PASSWORD);
        webServer.begin(jsonConnector);
    });

    if (!stateManager.readConfig()) {
        ERROR_PRINT("Configuration file error!!!\n", NULL);
//        return;
    }

    if (mqttService.connectToGsm()) {
        mqttService.connectMQTT();
    } else {
        ERROR_PRINT("Could not connect to GSM!!!\n", NULL);
        return;
    }

    mqttService.connectToGps();

    DEBUG_PRINT("Starting the app...\n", NULL);

    DefaultTasker.loopEvery("get-position", 2000, [] {
        mqttService.getCurrentPosition();
    });

    DefaultTasker.loopEvery("send-report", 10000, [] {
        if (!mqttService.sendReport()) {
            ERROR_PRINT("Send report failed!!!\n", NULL);
        }
    });

    jsonConnector.subscribe([](const JsonDocument &json) {
        DEBUG_PRINT("Configuration JSON received\n", NULL);
        File file = SPIFFS.open("/config.txt", "w+");
        if (!file) {
            ERROR_PRINT("Error opening config file for writing\n", NULL);
            return;
        }
        int usernameBytesWritten = file.println(json["username"].as<char *>());
        int passwordBytesWritten = file.println(json["password"].as<char *>());
        int tokenBytesWritten = file.println(json["token"].as<char *>());
        file.println(json["vehicleId"].as<long>());

        if (usernameBytesWritten > 0 && passwordBytesWritten > 0 && tokenBytesWritten > 0) {
            DEBUG_PRINT(
                    "%d bytes of username and %d bytes of password and %d bytes of access token was written to config file\n",
                    usernameBytesWritten, passwordBytesWritten, tokenBytesWritten);

        } else {
            ERROR_PRINT("Write to config file failed!\n", NULL);
        }

        file.close();
    });

    DEBUG_PRINT("Startup finished!\n", NULL);

    NetworkTasker.once("ap-off", [] {
        Tasker::sleep(120000);
        DEBUG_PRINT("Turning off AP...\n", NULL);
        AP::stop();
    });

}

void loop() {
    // no op - everything is handled by native tasks through Tasker
}