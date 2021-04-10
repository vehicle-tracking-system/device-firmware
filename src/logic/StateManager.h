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

    void printCurrentState() const;

    void toggleReconnecting();

    bool isReconnecting() const;

    void toggleGpsConnected();

    void setGpsState(bool state);

    bool isGpsConnected() const;

    bool readConfig();

    bool clearConfig();

    String getToken();

    void stopped();

    void moving();

    bool getIsMoving() const;

    long getVehicleId() const;

    void generateSessionId();

    uint8_t *getSessionId();

private:
    Preferences *preferences;
    String username;
    String password;
    String token;
    uint8_t sessionId[16];
    long vehicleId;
    bool reconnecting = false;
    bool gpsConnected = false;
    bool isMoving = false;
    unsigned int resetCounter;
    std::mutex state_mutex;
};


#endif //TRACKER_STATEMANAGER_H
