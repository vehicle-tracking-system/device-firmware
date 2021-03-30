#ifndef TRACKER_WIFICLIENT_H
#define TRACKER_WIFICLIENT_H

#include <Arduino.h>
#include <WiFi.h>

// TODO remove
class WifiClient {
public:
    static void begin(const char *ssid, const char *password) {

        Serial.print(F("Connecting to "));
        Serial.println(ssid);

        WiFi.begin(ssid, password);

        int i = 0;
        while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
            delay(1000);
            Serial.print(++i);
            Serial.print(' ');
        }

        Serial.println('\n');
        Serial.println(F("Connection established!"));
        Serial.print(F("IP address:\t"));
        Serial.println(WiFi.localIP());
    }
};

#endif //TRACKER_WIFICLIENT_H
