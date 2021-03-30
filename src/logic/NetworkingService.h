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
#include "KalmanFilter.h"
#include <ArduinoJson.h>
#include <mutex>
#include <deque>
#include <ctime>

class NetworkingService {

public:
    explicit NetworkingService(StateManager &stateManager);

    void begin();

    bool isModemConnected();

    bool isMqttConnected();

    bool connectToGsm();

    bool connectToGps();

    void connectMQTT();

    bool reconnect();

    bool getCurrentPosition();

    int buildReport(_protocol_Report *report);

    bool sendReport();

    bool sendReport(_protocol_Report &report);

private:
    void retrySendReport();

    bool sendToMqtt(_protocol_Report *report);

    StateManager *stateManager;
    Accelerometer *accelerometer;
    KalmanFilter *kalmanFilter = new KalmanFilter(10);
    TinyGsm modem = TinyGsm(SERIALGSM);
    TinyGsmClient gsmClient = TinyGsmClient(modem);
    SSLClient gsmClientSSL = SSLClient(&gsmClient);
    PubSubClient mqttClient = PubSubClient(MQTT_HOST, MQTT_PORT, gsmClientSSL);
    std::queue<_protocol_TrackerPosition> positionBuffer;
    std::queue<_protocol_Report> reportBuffer;
    std::mutex modem_mutex;
    std::mutex buffer_mutex;
    std::mutex report_buffer_mutex;
};


#endif //TRACKER_MQTTSERVICE_H
