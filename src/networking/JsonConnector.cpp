//Inspired by: https://github.com/jendakol/bomb-game/blob/0c25175c45a22eec3f1b15a869de26f3343957da/src/networking/JsonConnector.cpp
#include "JsonConnector.h"

#include <Constants.h>

JsonConnector::JsonConnector() {
    webSocket = new AsyncWebSocket(WEBSOCKET_PATH);

    webSocket->onEvent(
            [this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
                onWsEvent(type, data, len);
            }
    );
}

void JsonConnector::bind(AsyncWebServer &webServer) {
    webServer.addHandler(webSocket);
}

void JsonConnector::onWsEvent(AwsEventType type, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.println(F("Websocket client connection received"));
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.println(F("Websocket client disconnected"));
    } else if (type == WS_EVT_DATA) {
        Serial.print(F("Websocket data received: "));

        StaticJsonDocument<1024> json;
        deserializeJson(json, data, len);
        uint8_t test[2048];
        serializeJson(json, test, 2048);
        DEBUG_PRINT("%s\n", test);
        for (auto &callback : callbacks)
            callback(json);
    }
}

void JsonConnector::subscribe(const std::function<void(const JsonDocument &)> &callback) {
    DEBUG_PRINT("New subscription\n", NULL);
    callbacks.push_back(callback);
}

void JsonConnector::send(const JsonDocument &json) {
    size_t len = serializeJson(json, buff);
    this->webSocket->cleanupClients();
    if(this->webSocket->count())
        webSocket->textAll(buff, len);
}
