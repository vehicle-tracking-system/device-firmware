#include "GSM.h"

GSM::GSM(StateManager &stateManager) {
    this->stateManager = &stateManager;
}

bool GSM::begin() {
    if (!connectToGSM()) return false;
    connectToMQTT();
    if (!connectToGPS()) return false;
    return true;
}

bool GSM::isModemConnected() {
    std::lock_guard<std::mutex> lg(gsm_mutex);
    return modem.isNetworkConnected() && modem.isGprsConnected();
}

bool GSM::connectToGSM() {
    std::lock_guard<std::mutex> lg(gsm_mutex);
    if (stateManager->isReconnecting()) return false;
    stateManager->toggleReconnecting();

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
        DEBUG_PRINT("Waiting for network...\n", NULL);
        if (!modem.waitForNetwork()) {
            ERROR_PRINT(" fail\n", NULL);
            Tasker::yield();
            continue;
        }
        DEBUG_PRINT(" OK\n", NULL);
        Tasker::yield();

        DEBUG_PRINT("Connecting to %s\n", APN_HOST);

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

bool GSM::getCurrentPosition(_protocol_TrackerPosition *position) {
    if (!isModemConnected()) {
        ERROR_PRINT("Modem is not connected\n", NULL);
        return false;
    }
    DEBUG_PRINT("Getting current position...\n", NULL);
    float lat, lon, speed, alt, accuracy;
    int vsat, usat;
    tm rawTime = {};
    std::lock_guard<std::mutex> lg(gsm_mutex);
    if (modem.getGPS(&lat, &lon, &speed, &alt, &vsat, &usat, &accuracy, &rawTime.tm_year, &rawTime.tm_mon,
                     &rawTime.tm_mday, &rawTime.tm_hour, &rawTime.tm_min, &rawTime.tm_sec)) {
        DEBUG_PRINT("REAL   lat: %f, lng: %f\n", lat, lon);
        if (speed < SPEED_LOWER_LIMIT || speed > SPEED_UPPER_LIMIT) {
            // Speed is to slow or high to be relevant
            DEBUG_PRINT("Speed (%f) is to slow or high to be relevant. Accuracy: %f\n", speed, accuracy);
            return false;
        }
        if (abs(lat) > 90 || abs(lon) > 180) {
            // Position is out of range
            DEBUG_PRINT("Invalid position read\n", NULL);
            return false;
        }
        if (accuracy > MINIMAL_ACCURACY) {
            // Accuracy is below the minimal threshold
            DEBUG_PRINT("Accuracy is to low: %f, using GSM position\n", accuracy);
            return false;
        }
        rawTime.tm_year -= 1900;
        rawTime.tm_mon -= 1;
        long timestamp = mktime(&rawTime);

        DEBUG_PRINT("Act position at %d:%d:%d is %f %f, speed: %f, timestamp: %lu, accuracy: %f\n", rawTime.tm_hour,
                    rawTime.tm_min,
                    rawTime.tm_sec, lat, lon, speed, timestamp, accuracy);
        position->speed = speed;
        position->latitude = lat;
        position->longitude = lon;
        position->timestamp = timestamp;
        return true;
    } else {
        DEBUG_PRINT("GPS position not fixed\n", NULL);
        return false;
    }
}

bool GSM::connectToGPS() {
    std::lock_guard<std::mutex> lg(gsm_mutex);
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
    this->stateManager->setGpsState(true);
    DEBUG_PRINT("GPS position fixed: %f %f\n", lat, lon);

    stateManager->toggleReconnecting();
    return true;
}

bool GSM::reconnect() {
    while (!(isModemConnected() && isMqttConnected())) {
        if (!isModemConnected()) {
            DEBUG_PRINT("Reconnecting modem...\n", NULL);
            Tasker::yield();
            if (!connectToGSM()) {
                return false;
            }
        }

        if (connectToGSM() && !isMqttConnected()) {
            DEBUG_PRINT("Reconnecting MQTT...\n", NULL);
            Tasker::yield();
            this->connectToMQTT();
        }
    }

    return isModemConnected() && isMqttConnected();
}

bool GSM::isMqttConnected() {
    std::lock_guard<std::mutex> lg(gsm_mutex);
    return mqttClient.connected();
}

void GSM::connectToMQTT() {
    std::lock_guard<std::mutex> lg(gsm_mutex);

    if (stateManager->isReconnecting()) return;
    stateManager->toggleReconnecting();

    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    mqttClient.setKeepAlive(300);

    // Loop until we're reconnected
    while (!mqttClient.connected()) {
        Tasker::yield();
        // Create a random client ID
        String clientId = "TRACKER-";
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

void GSM::retrySendReport() {
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

bool GSM::sendToMqtt(_protocol_Report *report) {
    uint8_t buffer[1024];
    size_t message_length;
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    bool status = pb_encode(&stream, protocol_Report_fields, report);

    if (!status) {
        ERROR_PRINT("Could not encode status report!", NULL);
        return false;
    }

    message_length = stream.bytes_written;
    std::lock_guard<std::mutex> lg(gsm_mutex);
    bool published = mqttClient.publish(MQTT_TOPIC, buffer, message_length, true);
    DEBUG_PRINT("Publish %d bytes to MQTT end with result: %d\n", message_length, published);
    return true;
}

bool GSM::sendReport() {
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

int GSM::buildReport(_protocol_Report *report) {
    DEBUG_PRINT("Building report...\n", NULL);
    {
        std::lock_guard<std::mutex> lg(buffer_mutex);
        if (this->positionBuffer.empty()) return 0;
    }

    strcpy(report->token, stateManager->getToken().c_str());
    report->isMoving = true;
    report->vehicleId = this->stateManager->getVehicleId();
    memcpy(&report->sessionId.bytes, this->stateManager->getSessionId(), 16);
    report->sessionId.size = 16;

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

bool GSM::enqueueNewPosition() {
    _protocol_TrackerPosition position = protocol_TrackerPosition_init_zero;
    if (!this->getCurrentPosition(&position)) {
        return false;
    }
    std::lock_guard<std::mutex> lg(buffer_mutex);
    positionBuffer.push(position);
    return true;
}
