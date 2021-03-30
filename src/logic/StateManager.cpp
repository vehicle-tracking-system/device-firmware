#include "StateManager.h"

#include <utility>

void StateManager::printCurrentState() const {
    Serial.printf(
            "Reconnecting = %d Username = %s Password = %s\n",
            this->reconnecting, this->username.c_str(), this->password.c_str()
    );
}

void StateManager::toggleReconnecting() {
    this->reconnecting = !this->reconnecting;
}

bool StateManager::isReconnecting() const {
    return this->reconnecting;
}

bool StateManager::readConfig() {
    File file = SPIFFS.open("/config.txt", "r");
    if (!file.available()) {
        DEBUG_PRINT("Config file can not be open for reading\n", NULL);
        file.close();
        return false;
    }
    this->username = file.readStringUntil('\r');
    file.read();
    this->password = file.readStringUntil('\r');
    file.read();
    this->token = file.readStringUntil('\r');
    file.read();
    this->vehicleId = file.readStringUntil('\r').toInt();
    DEBUG_PRINT("Config:\n\tusername: %s\n\tpassword: %s\n\ttoken: %s\n\tvehicleId: %lu\n", this->username.c_str(),
                this->password.c_str(), this->token.c_str(), this->vehicleId);
    file.close();
    return true;
}

String StateManager::getToken() {
    return this->token;
}

void StateManager::setToken(String newToken) {
    std::lock_guard<std::mutex> lg(token_mutex);
    token = std::move(newToken);
}

void StateManager::toggleGpsConnected() {
    this->gpsConnected = !this->gpsConnected;
}

bool StateManager::isGpsConnected() const {
    return this->gpsConnected;
}

void StateManager::setGpsState(bool state) {
    this->gpsConnected = state;

}

String StateManager::getUsername() {
    return this->username;
}

String StateManager::getPassword() {
    return this->password;
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
