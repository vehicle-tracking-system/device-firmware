#include <mutex>
#include "NetworkingService.h"

#include <proto/pb_encode.h>

NetworkingService::NetworkingService(StateManager &stateManager) {
    this->stateManager = &stateManager;
    this->accelerometer = new Accelerometer();
}

void NetworkingService::begin() {
    this->accelerometer->begin();
}

bool NetworkingService::isModemConnected() {
    std::lock_guard<std::mutex> lg(modem_mutex);
    return modem.isNetworkConnected() && modem.isGprsConnected();
}

bool NetworkingService::isMqttConnected() {
    std::lock_guard<std::mutex> lg(modem_mutex);
    return mqttClient.connected();
}

bool NetworkingService::connectToGsm() {
    std::lock_guard<std::mutex> lg(modem_mutex);

    if (stateManager->isReconnecting()) return false;
    stateManager->toggleReconnecting();

    Tasker::sleep(55);
    modem.sleepEnable(false);

    bool modemConnected = false;

    DEBUG_PRINT("Initializing modem...\n", NULL);
    TinyGsmAutoBaud(SERIALGSM, 9600, 57600);

    if (!modem.init()) {
        ERROR_PRINT("Modem init failed!!!\n", NULL);
        // TODO handle
        return false;
    }

    DEBUG_PRINT("Modem: %s\n", modem.getModemInfo().c_str());

    Tasker::yield();

    while (!modemConnected) {
        DEBUG_PRINT("Waiting for network...", NULL);
        if (!modem.waitForNetwork()) {
            ERROR_PRINT(" fail\n", NULL);
            Tasker::yield();
            continue;
        }
        DEBUG_PRINT(" OK\n", NULL);
        Tasker::yield();

        DEBUG_PRINT("Connecting to %s", APN_HOST);

        if (!modem.gprsConnect(APN_HOST, APN_USER, APN_PASS)) {
            ERROR_PRINT(" fail\n", NULL);
            Tasker::yield();
            continue;
        }
        Tasker::yield();

        modemConnected = true;
        DEBUG_PRINT(" OK\n", NULL);
    }

    stateManager->toggleReconnecting();
    return true;
}

void NetworkingService::connectMQTT() {
    std::lock_guard<std::mutex> lg(modem_mutex);

    if (stateManager->isReconnecting()) return;
    stateManager->toggleReconnecting();

    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    mqttClient.setKeepAlive(300);

    // Loop until we're reconnected
    while (!mqttClient.connected()) {
        Tasker::yield();
        // Create a random client ID
        String clientId = "ESPClient-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        DEBUG_PRINT("Attempting MQTT connection...", NULL);
        mqttClient.disconnect();
        if (mqttClient.connect(clientId.c_str())) {
            DEBUG_PRINT(" connected to %s\n", MQTT_HOST);
        } else {
            ERROR_PRINT("failed, rc=%d try again in 5 seconds\n", mqttClient.state());
            // Wait 5 seconds before retrying
            Tasker::sleep(5000);
        }
    }

    stateManager->toggleReconnecting();
}

bool NetworkingService::connectToGps() {
    std::lock_guard<std::mutex> lg(modem_mutex);
    if (stateManager->isReconnecting()) return false;
    stateManager->toggleReconnecting();

    if (!modem.enableGPS()) {
        ERROR_PRINT("GPS error.\n", NULL);
        return false;
    }

    DEBUG_PRINT("GPS enabled. Waiting for signal...\n", NULL);
    float lat, lon, speed, alt, accuracy;
    int vsat, usat;
    while (!modem.getGPS(&lat, &lon, &speed, &alt, &vsat, &usat, &accuracy)) {
        Tasker::sleep(1000);
    }

    kalmanFilter->SetState(lat, lon, accuracy, millis());

    DEBUG_PRINT("GPS position fixed: %f %f\n", lat, lon);

    return true;
}

bool NetworkingService::getCurrentPosition() {
    if (!isModemConnected()) return false;
    accelerometer->read();
    std::lock_guard<std::mutex> lg(modem_mutex);
    DEBUG_PRINT("Getting current position...\n", NULL);
    float lat, lon, speed, alt, accuracy;
    int vsat, usat, year, month, day, hour, minute, second;
    if (modem.getGPS(&lat, &lon, &speed, &alt, &vsat, &usat, &accuracy, &year, &month, &day, &hour, &minute, &second)) {
        DEBUG_PRINT("Acceleration: %f\n", accelerometer->getAcceleration());
        kalmanFilter->Process(lat, lon, accuracy, millis());
        DEBUG_PRINT("KALMAN lat: %f, lng: %f\n", kalmanFilter->get_lat(), kalmanFilter->get_lng());
        DEBUG_PRINT("REAL   lat: %f, lng: %f\n", lat, lon);
        if (speed < SPEED_LOWER_LIMIT || speed > SPEED_UPPER_LIMIT) {
            // Speed is to slow or high to be relevant
            DEBUG_PRINT("Speed (%f) is to slow or high to be relevant. Accuracy: %f\n", speed, accuracy);
            return false;
        }
        // TODO neco lepsiho ??
        if (accuracy > 4) {
            DEBUG_PRINT("Accuracy is to low: %f, using GSM position\n", accuracy);
            // TOHLE SPADNE, TAK MAM SMULU :-(
//            modem.getGsmLocation(&lat, &lon, &accuracy, &year, &month, &day, &hour, &minute, &second);
            return false;
        }
        std::lock_guard<std::mutex> bg(buffer_mutex);
        struct tm t;
        time_t time;
        t.tm_year = year - 1900;
        t.tm_mon = month - 1;
        t.tm_mday = day;
        t.tm_hour = hour;
        t.tm_min = minute;
        t.tm_sec = second;
        t.tm_isdst = -1;
        time = mktime(&t);

        DEBUG_PRINT("Act position at %d:%d:%d is %f %f, speed: %f, timestamp: %lu, accuracy: %f\n", hour, minute,
                    second, lat, lon,
                    speed, time, accuracy);
        _protocol_TrackerPosition position = protocol_TrackerPosition_init_zero;
        position.speed = speed;
        position.latitude = lat;
        position.longitude = lon;
        position.timestamp = time;
        position.track = 1;
        position.vehicleId = stateManager->getVehicleId();
        positionBuffer.push(position);
        return true;
    } else {
        DEBUG_PRINT("GPS position not fixed\n", NULL);
        return false;
    }
}

bool NetworkingService::sendReport() {
    if (!(isModemConnected() && isMqttConnected())) {
        ERROR_PRINT("Not connected - could not report state\n", NULL);
        reconnect();
        return false;
    }
    DEBUG_PRINT("Sending report...\n", NULL);

    _protocol_Report report = protocol_Report_init_zero;
    int positionsToWrite = buildReport(&report);
    DEBUG_PRINT("Positions to write: %d\n", report.positions_count);
    if (positionsToWrite == 0) {
        DEBUG_PRINT("Nothing to send\n", NULL);
        return true;
    }

    bool published = sendToMqtt(&report);
    DEBUG_PRINT("IsPublished: %d\n", published);
    if (!published) {
        std::lock_guard<std::mutex> lg(report_buffer_mutex);
        this->reportBuffer.push(report);
        DefaultTasker.once("resend", [this] {
            Tasker::sleep(500);
            retrySendReport();
        });
        return false;
    }
    return true;
}

bool NetworkingService::reconnect() {
    while (!(isModemConnected() && isMqttConnected())) {
        if (!isModemConnected()) {
            DEBUG_PRINT("Reconnecting modem...\n", NULL);
            Tasker::yield();
            if (!this->connectToGsm()) {
                return false;
            }
        }

        if (this->connectToGsm() && !this->isMqttConnected()) {
            DEBUG_PRINT("Reconnecting MQTT...\n", NULL);
            Tasker::yield();
            this->connectMQTT();
        }
    }

    return isModemConnected() && isMqttConnected();
}

void NetworkingService::retrySendReport() {
    std::lock_guard<std::mutex> lg(report_buffer_mutex);
    if (reportBuffer.empty()) return;
    _protocol_Report report = reportBuffer.front();
    if (sendToMqtt(&report)) {
        reportBuffer.pop();
    } else {
        DefaultTasker.once("resend-again", [this] {
            Tasker::sleep(500);
            retrySendReport();
        });
        ERROR_PRINT("Error resend report via MQTT\n", NULL);
    }
}

int NetworkingService::buildReport(_protocol_Report *report) {
    DEBUG_PRINT("Building report...\n", NULL);
    {
        std::lock_guard<std::mutex> lg(modem_mutex);
        if (this->positionBuffer.empty()) return 0;
    }

    strcpy(report->token, stateManager->getToken().c_str());
    report->isMoving = true;
    int positionsToWrite = 0;

    std::lock_guard<std::mutex> lg(buffer_mutex);
    for (auto &p : report->positions) {
        if (this->positionBuffer.empty()) break;
        _protocol_TrackerPosition position = this->positionBuffer.front();
        this->positionBuffer.pop();
        p = position;
        positionsToWrite++;
    }

    report->positions_count = positionsToWrite;
    DEBUG_PRINT("Report build: positions to write: %d\n", report->positions_count);
    return positionsToWrite;
}

bool NetworkingService::sendToMqtt(_protocol_Report *report) {
    uint8_t buffer[1024];
    size_t message_length;
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    bool status = pb_encode(&stream, protocol_Report_fields, report);

    if (!status) {
        ERROR_PRINT("Could not encode status report!", NULL);
        return false;
    }

    message_length = stream.bytes_written;
    std::lock_guard<std::mutex> lg(modem_mutex);
    bool published = mqttClient.publish(MQTT_TOPIC, buffer, message_length, true);
    DEBUG_PRINT("Publish %d bytes to MQTT end with result: %d\n", message_length, published);
    return true;
}
