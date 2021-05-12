#include "Accelerometer.h"

Accelerometer::Accelerometer(StateManager &stateManager) {
    this->stateManager = &stateManager;
}

bool Accelerometer::begin() {
    if (!Wire.begin())
        return false;
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);
    delay(50);
    return true;
}

bool Accelerometer::updateAcceleration() {
    int16_t accelerometer_x, accelerometer_y, accelerometer_z;
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, 7 * 2, true);

    accelerometer_x = Wire.read() << 8 | Wire.read();
    accelerometer_y = Wire.read() << 8 | Wire.read();
    accelerometer_z = Wire.read() << 8 | Wire.read();
    Wire.read() << 8 | Wire.read();
    Wire.read() << 8 | Wire.read();
    Wire.read() << 8 | Wire.read();
    Wire.read() << 8 | Wire.read();
    if (accelerometer_x == -1){
        ERROR_PRINT("Accelerometer x coordinates is not valid %d\n", accelerometer_x);
        return false;
    }
    if (accelerometer_y == -1) {
        ERROR_PRINT("Accelerometer y coordinates is not valid %d\n", accelerometer_y);
        return false;
    }
    if (accelerometer_z == -1) {
        ERROR_PRINT("Accelerometer z coordinates is not valid %d\n", accelerometer_z);
        return false;
    }
    double acc = sqrt(
            accelerometer_x * accelerometer_x + accelerometer_y * accelerometer_y + accelerometer_z * accelerometer_z);
    double act = abs(this->acceleration - acc);
    this->acceleration = acc;

    DEBUG_PRINT("Acceleration: x %hd, y %hd, z %hd -- %f %f\n", accelerometer_x, accelerometer_y, accelerometer_z, acc,
                act);
//    stateManager->setAcc(act);
    if (this->stateManager->moving()) {
        if (act < MOVEMENT_THRESHOLD) {
            if (counter > 10) {
                this->stateManager->setMoving(false);
                counter = 0;
            }
            counter++;
        } else counter = 0;
    }

    if (!this->stateManager->moving()) {
        if (act >= MOVEMENT_THRESHOLD) {
            if (counter > 5) {
                this->stateManager->setMoving(true);
                counter = 0;
            }
            counter++;
        } else counter = 0;
    }

    return true;
}
