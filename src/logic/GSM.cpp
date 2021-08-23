#include "GSM.h"

#include <cmath>

GSM::GSM(StateManager &stateManager) {
    this->stateManager = &stateManager;
}

bool GSM::begin() {
    restartGSM();
    if (!connectToGSM()) return false;
    if (!connectToGPS()) return false;
    connectToMQTT();
    NetworkTasker.loopEvery("mqtt", 100, [this] {
        std::lock_guard<std::recursive_mutex> lg(gsm_mutex);
        mqttClient.loop();
    });
    return true;
}

bool GSM::isModemConnected() {
    std::lock_guard<std::recursive_mutex> lg(gsm_mutex);
    return modem.isNetworkConnected() && modem.isGprsConnected();
}

bool GSM::connectToGSM() {
    std::lock_guard<std::recursive_mutex> lg(gsm_mutex);
    if (stateManager->isReconnecting()) return false;
    stateManager->setTextState("Connecting to GSM...");
    stateManager->toggleReconnecting();

    modem.sleepEnable(false);

    bool modemConnected = false;

    DEBUG_PRINT("Initializing modem...\n", NULL);
    TinyGsmAutoBaud(SERIALGSM, 115200, 115200);
//    SERIALGSM.begin(9600);
    if (!modem.init()) {
        ERROR_PRINT("Modem init failed!!!\n", NULL);
        stateManager->setError();
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

        DEBUG_PRINT("Connecting to %s\n", stateManager->apnHost().c_str());

        if (!modem.gprsConnect(stateManager->apnHost().c_str(), stateManager->apnUser().c_str(),
                               stateManager->apnPass().c_str())) {
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
        this->stateManager->setTextState("Modem is not connected");
        ERROR_PRINT("Modem is not connected\n", NULL);
        return false;
    }
    DEBUG_PRINT("Getting current position...\n", NULL);
    float lat, lng, speed, alt, accuracy;
    int vsat, usat;
    tm rawTime = {};
    std::lock_guard<std::recursive_mutex> lg(gsm_mutex);
    if (modem.getGPS(&lat, &lng, &speed, &alt, &vsat, &usat, &accuracy, &rawTime.tm_year, &rawTime.tm_mon,
                     &rawTime.tm_mday, &rawTime.tm_hour, &rawTime.tm_min, &rawTime.tm_sec)) {
        auto distance = 6378.388 * acos(sin(deg2rad(lat)) * sin(deg2rad(stateManager->lat())) + cos(deg2rad(lat)) * cos(
                deg2rad(stateManager->lat())) * cos(deg2rad(stateManager->lng() - lng)));
        DEBUG_PRINT("CURRENT lat: %f, lng: %f, distance: %f\n", lat, lng, distance);
        stateManager->setTextState(
                String("Position sample ") + lat + " " + lng + " " + speed + " kph");
#ifdef USE_ACCELEROMETER
        if (!this->stateManager->moving()) {
            // Tracker is not moving
            if (speed < SPEED_LOWER_LIMIT) {
                DEBUG_PRINT("Tracker is not moving\n", NULL);
                return false;
            }
        }
#else
        if (speed < SPEED_LOWER_LIMIT || speed > SPEED_UPPER_LIMIT) {
            // Speed is to slow or high to be relevant
            DEBUG_PRINT("Speed (%f) is to slow or high to be relevant. Accuracy: %f\n", speed, accuracy);
            return false;
        }
#endif
        if (abs(lat) > 90 || abs(lng) > 180) {
            // Position is out of range
            DEBUG_PRINT("Invalid position read\n", NULL);
            return false;
        }
        if (accuracy > MINIMAL_ACCURACY) {
            // Accuracy is below the minimal threshold
            DEBUG_PRINT("Accuracy is to low: %f, using GSM position\n", accuracy);
            return false;
        }

        if (stateManager->isGpsConnected() && MAX_DISTANCE_BETWEEN_TWO_POSITIONS < distance) {
            DEBUG_PRINT("Distance between two positions is too long: %f\n", distance);
            return false;
        }

        stateManager->setGpsState(true);
        stateManager->setPosition(lat, lng, speed);
        rawTime.tm_year -= 1900;
        rawTime.tm_mon -= 1;
        long timestamp = mktime(&rawTime);

        DEBUG_PRINT("Act position at %d:%d:%d is %f %f, speed: %f, timestamp: %lu, accuracy: %f\n", rawTime.tm_hour,
                    rawTime.tm_min,
                    rawTime.tm_sec, lat, lng, speed, timestamp, accuracy);
        position->speed = speed;
        position->latitude = lat;
        position->longitude = lng;
        position->timestamp = timestamp;
        return true;
    } else {
        DEBUG_PRINT("GPS position not fixed\n", NULL);
        return false;
    }
}

bool GSM::connectToGPS() {
    std::lock_guard<std::recursive_mutex> lg(gsm_mutex);
    if (stateManager->isGpsReconnecting()) return false;
    stateManager->toggleGpsReconnecting();
    stateManager->setTextState("Waiting for GSP...");

    if (!modem.enableGPS()) {
        ERROR_PRINT("GPS error.\n", NULL);
        stateManager->setError();
        return false;
    }

    DEBUG_PRINT("GPS enabled. Waiting for signal...\n", NULL);
    float lat, lon, speed, alt, accuracy;
    int vsat, usat;
    while (!modem.getGPS(&lat, &lon, &speed, &alt, &vsat, &usat, &accuracy)) {
        if (this->isModemConnected()) {
            String gps_raw = modem.getGPSraw();
            DEBUG_PRINT("Visible satellites: %d\n\tUsed satellites: %d\n\tRaw: %s\n", vsat, usat, gps_raw.c_str());
            Tasker::sleep(1500);
        } else {
            this->reconnect();
        }
    }
    DEBUG_PRINT("\n", NULL);
    this->stateManager->setPosition(lat, lon, speed);
    DEBUG_PRINT("GPS position fixed: %f %f\n", lat, lon);

    stateManager->toggleGpsReconnecting();
    return true;
}

bool GSM::reconnect() {
    startGSM();
    std::lock_guard<std::recursive_mutex> lg(gsm_mutex);
    stateManager->setTextState("Reconnecting...");
    while (!(isModemConnected() && isMqttConnected())) {
        if (!isModemConnected()) {
            stateManager->setTextState("Reconnecting modem...");
            DEBUG_PRINT("Reconnecting modem...\n", NULL);
            Tasker::yield();
            if (!connectToGSM()) {
                return false;
            }
        }

        if (!isMqttConnected()) {
            stateManager->setTextState("Reconnecting MQTT...");
            DEBUG_PRINT("Reconnecting MQTT...\n", NULL);
            Tasker::yield();
            this->connectToMQTT();
        }
        Tasker::yield();
    }

    return isModemConnected() && isMqttConnected();
}

bool GSM::isMqttConnected() {
    std::lock_guard<std::recursive_mutex> lg(gsm_mutex);
    return mqttClient.connected();
}

void GSM::connectToMQTT() {
    std::lock_guard<std::recursive_mutex> lg(gsm_mutex);

    if (stateManager->isReconnecting()) return;
    stateManager->toggleReconnecting();
    String domain = stateManager->mqttHost();
    mqttClient.setServer(domain.c_str(), stateManager->mqttPort());
//    mqttClient.setKeepAlive((REPORT_FREQUENCE * 2) / 1000);

    int errorAttempts = 0;
    // Loop until we're reconnected
    while (!mqttClient.connected()) {
        Tasker::yield();
        // Create a random client ID
        String clientId = "TRACKER-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        DEBUG_PRINT("Attempting MQTT connection...", NULL);
        mqttClient.disconnect();
        if (mqttClient.connect(clientId.c_str(),
                               stateManager->mqttUsername().c_str(),
                               stateManager->mqttPassword().c_str())) {
            stateManager->setTextState("connected to MQTT");
            DEBUG_PRINT(" connected to %s\n", stateManager->mqttHost().c_str());
        } else {
            ERROR_PRINT("failed, rc=%d try again in 5 seconds\n", mqttClient.state());
            errorAttempts++;
            if (errorAttempts > 5) stateManager->setError("MQTT connection error.");
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
    DEBUG_PRINT("RESEND REPORT\n", NULL);
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
        ERROR_PRINT("Could not encode status report!\n", NULL);
        return false;
    }
    if (mqttClient.state() != 0) {
        ERROR_PRINT("MQTT client error: %d\n", mqttClient.state());
        return false;
    }
    LedController::blink(LedController::GREEN, 2);
    message_length = stream.bytes_written;
    std::lock_guard<std::recursive_mutex> lg(gsm_mutex);
    bool published = mqttClient.publish(stateManager->mqttTopic().c_str(), buffer, message_length, true);
    this->stateManager->setTextState(String("published ") + message_length + " bytes to MQTT");
    DEBUG_PRINT("Publish %d bytes to MQTT end with result: %d\n", message_length, published);
    return published;
}

bool GSM::sendReport() {
    std::lock_guard<std::recursive_mutex> lg(gsm_mutex);
    DEBUG_PRINT("MQTT state = %d\n", mqttClient.state());
    this->stateManager->setTextState(String("MQTT state = ") + mqttClient.state());
    if (!(isModemConnected() && isMqttConnected())) {
        ERROR_PRINT("Not connected - could not report state\n", NULL);
        DefaultTasker.once("", [this] { reconnect(); });
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
        std::lock_guard<std::mutex> ll(report_buffer_mutex);
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
        ERROR_PRINT("Get new position error\n", NULL);
        return false;
    }
    LedController::blink(LedController::GREEN);
    std::lock_guard<std::mutex> lg(buffer_mutex);
    positionBuffer.push(position);
    return true;
}

void GSM::startGSM() {
    digitalWrite(POWER_PIN, HIGH);
}

void GSM::restartGSM() {
    digitalWrite(POWER_PIN, LOW);
    delay(200);
    digitalWrite(POWER_PIN, HIGH);
}

double GSM::deg2rad(double deg) {
    return (deg * M_PI / 180);
}
