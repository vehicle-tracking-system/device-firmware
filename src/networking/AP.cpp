#include <WiFi.h>

#include <Constants.h>
#include <DNSServer.h>
#include "AP.h"

void AP::begin(const char *ssid, const char *password) {
    WiFi.disconnect(true);

    IPAddress apIP(8, 8, 8, 8);
    IPAddress gtw(8, 8, 8, 8);
    IPAddress netMsk(255, 255, 255, 0);

    DEBUG_PRINT("Configuring access point...\n",NULL);
    WiFi.softAP(ssid, password);
    delay(100);
    WiFi.softAPConfig(apIP, gtw, netMsk);

    auto *dnsServer = new DNSServer();

    /* Setup the DNS server redirecting all the domains to the apIP */
    dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer->start(DNS_PORT, "*", apIP);

    NetworkTasker.loop("dns-server", [dnsServer] {
        dnsServer->processNextRequest();
        taskYIELD();
    });

    delay(500);
    DEBUG_PRINT("AP IP address: %s\n", WiFi.softAPIP().toString().c_str());
}

void AP::stop() {
    WiFi.enableAP(false);
    WiFi.softAPdisconnect(true);
}
