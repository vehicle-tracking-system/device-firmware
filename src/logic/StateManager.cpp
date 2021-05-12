#include "StateManager.h"

#include <utility>

StateManager::StateManager(Preferences &preferences) {
    this->preferences = &preferences;
}

bool StateManager::begin() {
    resetCounter = preferences->getUInt("resets", 0);
    DEBUG_PRINT("Total resets: %u\n", resetCounter);
//    if (resetCounter >= 4) {
//        clearConfig();
//        return false;
//    }
//    preferences->putUInt("resets", resetCounter + 1);
//    DefaultTasker.once("resetcnt", [this]() {
//        Tasker::sleep(10000);
//        DEBUG_PRINT("Resetting counter of resets!\n", NULL);
//        preferences->putUInt("resets", 0);
//    });
    generateSessionId();
    readConfig();
    return true;
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
    this->mHost = preferences->getString("mqttHost", "N/A");
    this->mPort = preferences->getInt("mqttPort", 8883);
    this->mTopic = preferences->getString("mqttTopic", "tracker");
    this->mUsername = preferences->getString("mqttUsername", "");
    this->mPassword = preferences->getString("mqttPassword", "");
    this->aHost = preferences->getString("apnHost", "");
    this->aUser = preferences->getString("apnUser", "");
    this->aPass = preferences->getString("apnPass", "");
    DEBUG_PRINT(
            "Config:\n\ttoken: %s\n\tvehicleId: %lu\n\tmqtt host: %s\n\tmqtt port: %d\n\tmqtt topic: %s\n\tmqtt username: %s\n\tmqtt password: %s\n\tapn host: %s\n\tapn user: %s\n\tapn pass: %s\n",
            token.c_str(), vehicleId, mHost.c_str(), mPort, mTopic.c_str(), mUsername.c_str(), mPassword.c_str(),
            aHost.c_str(), aUser.c_str(), aPass.c_str());
    return !((token == "N/A") || (vehicleId == -1) || (mHost == "N/A"));
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

long StateManager::getVehicleId() const {
    return this->vehicleId;
}

uint8_t *StateManager::getSessionId() {
    return this->sessionId;
}

void StateManager::generateSessionId() {
    std::lock_guard<std::mutex> lg(state_mutex);
    DEBUG_PRINT("Generating new session ID\n", NULL);
    setTextState("Generating new session ID");
    esp_fill_random(this->sessionId, 16);
}

bool StateManager::clearConfig() {
    std::lock_guard<std::mutex> lg(state_mutex);
    return preferences->clear();
}

bool StateManager::isError() const {

    return error;
}

void StateManager::setError(String message) {
    this->error = true;
    this->stateMessage = std::move(message);
}

void StateManager::toggleGpsReconnecting() {
    this->gpsReconnecting = !this->gpsReconnecting;
}

bool StateManager::isGpsReconnecting() const {
    return this->gpsReconnecting;
}

String StateManager::textState() {
    return stateMessage;
}

void StateManager::setTextState(String state) {
    this->stateMessage = std::move(state);
    this->newState = true;
}

void StateManager::setPosition(float lat, float lng, float speed) {
    this->latitude = lat;
    this->longitude = lng;
    this->spd = speed;
}

float StateManager::lat() const {
    return latitude;
}

float StateManager::lng() const {
    return longitude;
}

bool StateManager::stateToSend() {
    bool tmp = newState;
    newState = false;
    return tmp;
}

float StateManager::speed() const {
    return spd;
}

String StateManager::mqttHost() const {
    return mHost;
}

int StateManager::mqttPort() const {
    return mPort;
}

String StateManager::mqttUsername() const {
    return mUsername;
}

String StateManager::mqttPassword() const {
    return mPassword;
}

String StateManager::mqttTopic() const {
    return mTopic;
}

void StateManager::setMoving(bool im) {
    this->isMoving = im;
}

bool StateManager::moving() const {
    return isMoving;
}

void StateManager::setAcc(double acc) {
    acceleration = acc;
    newState = true;
}

double StateManager::getAcc() {
    return acceleration;
}

String StateManager::apnHost() const {
    return aHost;
}

String StateManager::apnUser() const {
    return aUser;
}

String StateManager::apnPass() const {
    return aPass;
}
