//Inspired by: https://github.com/jendakol/bomb-game/blob/0c25175c45a22eec3f1b15a869de26f3343957da/src/networking/WebServer.h
#ifndef TRACKER_WEBSERVER_H
#define TRACKER_WEBSERVER_H

#include <vector>
#include <ESPAsyncWebServer.h>

#include "JsonConnector.h"
#include <Constants.h>

class WebServer {
public:
    WebServer();

    void begin(JsonConnector &jsonConnector);

    void stop();

private:
    AsyncWebServer *webServer;
    bool send = false;

    static bool handleStaticFile(AsyncWebServerRequest *request) {
        String path = STATIC_FILES_PREFIX + request->url();

        if (path.endsWith("/")) path += F("index.html");

        String contentType = getContentType(path);
        String pathWithGz = path + ".gz";

        if (FileSystem.exists(pathWithGz) || FileSystem.exists(path)) {
            bool gzipped = false;

            if (FileSystem.exists(pathWithGz)) {
                gzipped = true;
                path += ".gz";
            }

            File file = FileSystem.open(path, "r");

            DEBUG_PRINT("Serving static file, path=%s size=%d content-type=%s", path, file.size(), contentType);

            AsyncWebServerResponse *response = request->beginResponse(
                    contentType,
                    file.size(),
                    [file](uint8_t *buffer, size_t maxLen, size_t total) mutable -> size_t {
                        int bytes = file.read(buffer, maxLen);

                        // close file at the end
                        if (bytes + total == file.size()) file.close();

                        return max(0, bytes); // return 0 even when no bytes were loaded
                    }
            );

            if (gzipped) {
                response->addHeader(F("Content-Encoding"), F("gzip"));
            }

            request->send(response);

            return true;
        }

        return false;
    }

    static String getContentType(const String &filename) {
        if (filename.endsWith(F(".htm"))) return F("text/html");
        else if (filename.endsWith(F(".html"))) return F("text/html");
        else if (filename.endsWith(F(".css"))) return F("text/css");
        else if (filename.endsWith(F(".js"))) return F("application/javascript");
        else if (filename.endsWith(F(".png"))) return F("image/png");
        else if (filename.endsWith(F(".gif"))) return F("image/gif");
        else if (filename.endsWith(F(".jpg"))) return F("image/jpeg");
        else if (filename.endsWith(F(".ico"))) return F("image/x-icon");
        else if (filename.endsWith(F(".xml"))) return F("text/xml");
        else if (filename.endsWith(F(".pdf"))) return F("application/x-pdf");
        else if (filename.endsWith(F(".zip"))) return F("application/x-zip");
        else if (filename.endsWith(F(".gz"))) return F("application/x-gzip");

        return F("text/plain");
    }
};


#endif //TRACKER_WEBSERVER_H
