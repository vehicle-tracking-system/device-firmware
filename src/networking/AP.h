#ifndef TRACKER_AP_H
#define TRACKER_AP_H


class AP {
public:
    static void begin(const char *ssid, const char *password);

    /**
     * Stop AP and WiFi
     * */
    static void stop();
};


#endif //TRACKER_AP_H
