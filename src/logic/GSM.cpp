#include "GSM.h"

GSM::GSM(StateManager &stateManager) {
    this->stateManager = &stateManager;
}

bool GSM::begin() {
    if (!connectToGSM()) return false;
    if (!connectToGPS()) return false;
    return true;
}

bool GSM::isModemConnected() {
    std::lock_guard<std::mutex> lg(modem_mutex);
    return modem.isNetworkConnected() && modem.isGprsConnected();
}

bool GSM::connectToGSM() {
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
    if (!isModemConnected()) return false;
    std::lock_guard<std::mutex> lg(modem_mutex);
    DEBUG_PRINT("Getting current position...\n", NULL);
    float lat, lon, speed, alt, accuracy;
    int vsat, usat;
    tm rawTime = {};
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

SSLClient &GSM::getSSLClient() {
    return this->gsmClientSSL;
}

bool GSM::connectToGPS() {
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
    this->stateManager->setGpsState(true);
    DEBUG_PRINT("GPS position fixed: %f %f\n", lat, lon);

    return true;
}

bool GSM::reconnect() {
    DEBUG_PRINT("Reconnecting modem...\n", NULL);
    Tasker::yield();
    if (!this->connectToGSM()) {
        ERROR_PRINT("Reconnecting modem failed... Check wiring\n", NULL);
        return false;
    }
    return isModemConnected();
}