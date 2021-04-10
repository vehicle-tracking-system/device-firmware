#include "StateManager.h"

#include <utility>

StateManager::StateManager(Preferences &preferences) {
    this->preferences = &preferences;
}

bool StateManager::begin() {
    resetCounter = preferences->getUInt("resets", 0);
    DEBUG_PRINT("Total resets: %u\n", resetCounter);
    if (resetCounter >= 4) {
        clearConfig();
        return true;
    }
    preferences->putUInt("resets", resetCounter + 1);
    DefaultTasker.once("resetcnt", [this]() {
        Tasker::sleep(10000);
        DEBUG_PRINT("Resetting counter of resets!\n", NULL);
        preferences->putUInt("resets", 0);
    });
    generateSessionId();
    readConfig();
}

void StateManager::printCurrentState() const {
    Serial.printf(
            "Reconnecting = %d Username = %s Password = %s\n",
            this->reconnecting, this->username.c_str(), this->password.c_str()
    );
}

void StateManager::toggleReconnecting() {
    std::lock_guard<std::mutex> lg(state_mutex);
    this->reconnecting = !this->reconnecting;
}

bool StateManager::isReconnecting() const {
    return this->reconnecting;
}

bool StateManager::readConfig() {
    std::lock_guard<std::mutex> lg(state_mutex);

    this->token = preferences->getString("token", "N/A");
    this->vehicleId = preferences->getLong("vehicleId", -1);
    DEBUG_PRINT("Config:\n\ttoken: %s\n\tvehicleId: %lu\n", token.c_str(), vehicleId);
    return !((token == "N/A") | (vehicleId == -1));
}

String StateManager::getToken() {
    return this->token;
}

void StateManager::toggleGpsConnected() {
    std::lock_guard<std::mutex> lg(state_mutex);
    this->gpsConnected = !this->gpsConnected;
}

bool StateManager::isGpsConnected() const {
    return this->gpsConnected;
}

void StateManager::setGpsState(bool state) {
    this->gpsConnected = state;
}

void StateManager::stopped() {
    this->isMoving = false;
}

void StateManager::moving() {
    this->isMoving = true;
}

long StateManager::getVehicleId() const {
    return this->vehicleId;
}

bool StateManager::getIsMoving() const {
    return this->isMoving;
}

uint8_t *StateManager::getSessionId() {
    return this->sessionId;
}

void StateManager::generateSessionId() {
    std::lock_guard<std::mutex> lg(state_mutex);
    esp_fill_random(this->sessionId, 16);
}

bool StateManager::clearConfig() {
    std::lock_guard<std::mutex> lg(state_mutex);
    return preferences->clear();
}
