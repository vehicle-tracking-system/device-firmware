#ifndef TRACKER_MQTTSERVICE_H
#define TRACKER_MQTTSERVICE_H

#include <Arduino.h>
#include <Constants.h>

#include <Tasker.h>
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include <ArduinoHttpClient.h>
#include "SSLClient/SSLClient.h"
#include "StateManager.h"
#include "Accelerometer.h"
#include "GSM.h"
#include <ArduinoJson.h>
#include <mutex>
#include <deque>
#include <ctime>

class NetworkingService {

public:
    explicit NetworkingService(StateManager &stateManager, GSM &gsm);

    /**
     * State manager and GSM MUST BE initialized before calling begin!
     * Connect to MQTT broker.
     * */
    void begin();

    /**
     * @return true if connected to MQTT, otherwise false
     * */
    bool isMqttConnected();

    /**
     * Connect to MQTT with random client ID (prefixed with `TRACKER-`).
     * This function is blocking. Waiting until connect to MQTT broker.
     * */
    void connectMQTT();

    /**
     * Restore GSM and MQTT connection
     *
     * @return true if reconnection was successful, otherwise false
     * */
    bool reconnect();

    bool getCurrentPosition();

    /**
     * Read new position from GSM and enqueue it to buffer.
     *
     * @return true if position was successfully read and push to buffer queue
     * */
    bool enqueueNewPosition();

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

private:
    void retrySendReport();

    bool sendToMqtt(_protocol_Report *report);

    StateManager *stateManager;
    Accelerometer *accelerometer;
    GSM *gsm;
    PubSubClient mqttClient;
    std::queue<_protocol_TrackerPosition> positionBuffer;
    std::queue<_protocol_Report> reportBuffer;
    std::mutex modem_mutex;
    std::mutex buffer_mutex;
    std::mutex report_buffer_mutex;
};


#endif //TRACKER_MQTTSERVICE_H
