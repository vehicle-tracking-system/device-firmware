//Inspired by: https://github.com/jendakol/bomb-game/blob/0c25175c45a22eec3f1b15a869de26f3343957da/src/networking/WebServer.cpp
#include "WebServer.h"

WebServer::WebServer() {
    webServer = new AsyncWebServer(HTTP_PORT);
}

void WebServer::begin(JsonConnector &jsonConnector) {
    webServer->onNotFound([](AsyncWebServerRequest *request) {
        if (WebServer::handleStaticFile(request)) return;

        request->send(404);
    });

    jsonConnector.bind(*webServer);

    webServer->begin();
}

void WebServer::stop() {
    webServer->end();
}
