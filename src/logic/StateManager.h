#ifndef TRACKER_STATEMANAGER_H
#define TRACKER_STATEMANAGER_H

#include <Arduino.h>
#include "Constants.h"
#include <proto/tracker.pb.h>
#include <queue>
#include <mutex>

class StateManager {
public:
    void printCurrentState() const;

    void toggleReconnecting();

    bool isReconnecting() const;

    void toggleGpsConnected();

    void setGpsState(bool state);

    bool isGpsConnected() const;

    bool readConfig();

    String getToken();

    void setToken(String newToken);

    String getUsername();

    String getPassword();

    void stopped();

    void moving();

    bool getIsMoving() const;

    long getVehicleId() const;
private:
    String username;
    String password;
    String token;
    long vehicleId;
    bool reconnecting = false;
    bool gpsConnected = false;
    bool isMoving = false;
    std::mutex token_mutex;
};


#endif //TRACKER_STATEMANAGER_H
