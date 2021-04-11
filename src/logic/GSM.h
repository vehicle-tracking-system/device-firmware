#ifndef TRACKER_GSM_H
#define TRACKER_GSM_H

#include <Arduino.h>
#include <Constants.h>

#include <TinyGsmClient.h>
#include <mutex>
#include <SSLClient/SSLClient.h>
#include <PubSubClient.h>
#include <proto/pb_encode.h>
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
     * This function is not synchronized.
     *
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
     * Restore GSM and MQTT connection
     *
     * @return true if reconnection was successful, otherwise false
     * */
    bool reconnect();

    /**
     * @return true if connected to MQTT, otherwise false
     * */
    bool isMqttConnected();

    /**
     * Build report from position in buffer.
     *
     * @return number of positions inside report
     * */
    int buildReport(_protocol_Report *report);

    /**
     * Send report to (prepared with `buildReport`) MQTT broker.
     * If sending failed, resend is handle internally.
     *
     * @return true if report was send successfully, otherwise false
     * */
    bool sendReport();

    /**
     * Read new position from GSM and enqueue it to buffer.
     *
     * @return true if position was successfully read and push to buffer queue
     * */
    bool enqueueNewPosition();

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

    /**
    * Connect to MQTT with random client ID (prefixed with `TRACKER-`).
    * This function is blocking. Waiting until connect to MQTT broker.
    * */
    void connectToMQTT();

    void retrySendReport();

    bool sendToMqtt(_protocol_Report *report);

    StateManager *stateManager;
    TinyGsm modem = TinyGsm(SERIALGSM);
    TinyGsmClient gsmClient = TinyGsmClient(modem);
    SSLClient gsmClientSSL = SSLClient(&gsmClient);
    PubSubClient mqttClient = PubSubClient(MQTT_HOST, MQTT_PORT, gsmClientSSL);
    std::queue<_protocol_TrackerPosition> positionBuffer;
    std::queue<_protocol_Report> reportBuffer;
    std::mutex gsm_mutex; // everything integrating with gsm module (mqttClient, gsmClient, gsmClientSSL) must be synchronized!
    std::mutex report_buffer_mutex;
    std::mutex buffer_mutex;
};

#endif //TRACKER_GSM_H
