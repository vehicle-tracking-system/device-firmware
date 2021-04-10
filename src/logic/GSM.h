#ifndef TRACKER_GSM_H
#define TRACKER_GSM_H

#include <Arduino.h>
#include <Constants.h>

#include <TinyGsmClient.h>
#include <mutex>
#include <SSLClient/SSLClient.h>
#include "StateManager.h"

/**
 * Providing API for communication with SIM808 (GSM/GPS) module.
 * */
class GSM {
public:
    explicit GSM(StateManager &stateManager);

    /**
     * Connect module to GSP and GPS.
     * This function is blocking! For more info look at `connectToGSM` and `connectToGPS`.
     * */
    bool begin();

    /**
     * @return true if modem is connected to network and GPRS, otherwise false
     * */
    bool isModemConnected();

    /**
     * Write actual module position to @arg trackerPosition.
     *
     * @return true if actual position was read successfully, otherwise false
     * */
    bool getCurrentPosition(_protocol_TrackerPosition *trackerPosition);

    /**
     *  Reconnect to GSM network and Internet
     * */
    bool reconnect();

    /**
     * @return GSM client wrapped in SSL.
     * */
    SSLClient &getSSLClient();

private:
    /**
     * Connect module to GSM network.
     * This function is blocking. Waiting until GSM module to connect to network.
     *
     * @return true if module was successfully connected to the network, otherwise false
     * */
    bool connectToGSM();

    /**
     * Connect module to GPS.
     * This function is blocking. Waiting until GSM module to connect to network.
     *
     * @return true if module was successfully connected to the GPS, otherwise false
     * */
    bool connectToGPS();

    StateManager *stateManager;
    TinyGsm modem = TinyGsm(SERIALGSM);
    TinyGsmClient gsmClient = TinyGsmClient(modem);
    SSLClient gsmClientSSL = SSLClient(&gsmClient);
    std::mutex modem_mutex;
};

#endif //TRACKER_GSM_H
