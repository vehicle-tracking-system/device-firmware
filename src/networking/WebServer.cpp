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
