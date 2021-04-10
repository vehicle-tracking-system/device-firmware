#include <mutex>
#include "NetworkingService.h"

#include <proto/pb_encode.h>

NetworkingService::NetworkingService(StateManager &stateManager, GSM &gsm) {
    this->stateManager = &stateManager;
    this->gsm = &gsm;
    this->accelerometer = new Accelerometer();
}

void NetworkingService::begin() {
    this->mqttClient = PubSubClient(MQTT_HOST, MQTT_PORT, this->gsm->getSSLClient());
    this->accelerometer->begin();
    connectMQTT();
}

bool NetworkingService::isMqttConnected() {
    std::lock_guard<std::mutex> lg(modem_mutex);
    return mqttClient.connected();
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

bool NetworkingService::getCurrentPosition() {
    _protocol_TrackerPosition position = protocol_TrackerPosition_init_zero;
    positionBuffer.push(position);
}

bool NetworkingService::sendReport() {
    if (!(gsm->isModemConnected() && isMqttConnected())) {
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
    while (!(gsm->isModemConnected() && isMqttConnected())) {
        if (!gsm->isModemConnected()) {
            DEBUG_PRINT("Reconnecting modem...\n", NULL);
            Tasker::yield();
            if (!gsm->reconnect()) return false;
        }

        if (!this->isMqttConnected()) {
            DEBUG_PRINT("Reconnecting MQTT...\n", NULL);
            Tasker::yield();
            this->connectMQTT();
        }
    }

    return gsm->isModemConnected() && isMqttConnected();
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

bool NetworkingService::enqueueNewPosition() {
    _protocol_TrackerPosition position = protocol_TrackerPosition_init_zero;
    if (!this->gsm->getCurrentPosition(&position)) {
        ERROR_PRINT("Enqueue new position error\n", NULL);
        return false;
    }
    std::lock_guard<std::mutex> lg(modem_mutex);
    positionBuffer.push(position);
    return true;
}
