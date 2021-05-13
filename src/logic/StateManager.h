#ifndef TRACKER_STATEMANAGER_H
#define TRACKER_STATEMANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include "Constants.h"
#include <proto/tracker.pb.h>
#include <queue>
#include <mutex>

class StateManager {
public:
    explicit StateManager(Preferences &preferences);

    bool begin();

    void setPosition(float lat, float lng, float speed);

    float lat() const;

    float lng() const;

    float speed() const;

    void printCurrentState() const;

    void toggleReconnecting();

    void toggleGpsReconnecting();

    bool isReconnecting() const;

    bool isGpsReconnecting() const;

    void toggleGpsConnected();

    void setGpsState(bool state);

    bool isGpsConnected() const;

    bool readConfig();

    bool clearConfig();

    String getToken();

    long getVehicleId() const;

    void generateSessionId();

    uint8_t *getSessionId();

    bool isError() const;

    void setError(String message = "");

    void setTextState(String state);

    String textState();

    bool stateToSend();

    String mqttHost() const;

    int mqttPort() const;

    String mqttTopic() const;

    String mqttUsername() const;

    String mqttPassword() const;

    String apnHost() const;

    String apnUser() const;

    String apnPass() const;

    void setMoving(bool im);

    bool moving() const;

    void setAcc(double acc);

    double getAcc();

private:
    Preferences *preferences;
    String username;
    String password;
    String token;
    uint8_t sessionId[16];
    String stateMessage;
    String mHost;
    int mPort;
    String mUsername;
    String mTopic;
    String mPassword;
    String aHost;
    String aUser;
    String aPass;
    bool newState = false;
    float latitude = 0;
    float longitude = 0;
    float spd = 0;
    double acceleration = 0;
    long vehicleId;
    bool error = false;
    bool reconnecting = false;
    bool gpsReconnecting = false;
    bool gpsConnected = false;
    bool redLed = false;
    bool greenLed = false;
    bool redIsBlinking = false;
    bool greenIsBlinking = false;
    unsigned int resetCounter;
    bool isMoving = false;
    std::mutex state_mutex;
};


#endif //TRACKER_STATEMANAGER_H
