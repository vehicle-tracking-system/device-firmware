//Inspired by: https://github.com/jendakol/bomb-game/blob/0c25175c45a22eec3f1b15a869de26f3343957da/src/networking/JsonConnector.h
#ifndef TRACKER_JSONCONNECTOR_H
#define TRACKER_JSONCONNECTOR_H

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

class JsonConnector {
public:
    JsonConnector();

    void bind(AsyncWebServer &webServer);

    void subscribe(const std::function<void(const JsonDocument &)> &callback);

    void send(const JsonDocument &json);

private:
    AsyncWebSocket *webSocket;
    char buff[1024];
    std::vector<std::function<void(const JsonDocument &)>> callbacks;

    void onWsEvent(AwsEventType type, uint8_t *data, size_t len);
};


#endif //TRACKER_JSONCONNECTOR_H
