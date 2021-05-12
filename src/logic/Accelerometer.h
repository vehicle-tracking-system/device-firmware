#ifndef TRACKER_ACCELEROMETER_H
#define TRACKER_ACCELEROMETER_H

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "Constants.h"
#include "StateManager.h"

class Accelerometer {
public:
    explicit Accelerometer(StateManager &stateManager);

    /**
     * Initialize communication with accelerometer via I2C bus.
     * @return true if initialization was successful, otherwise false
     */
    bool begin();

    /**
     * Read data from accelerometer and save them to the state manager.
     * @return true if data read successfully, otherwise false
     */
    bool updateAcceleration();

private:
    StateManager *stateManager;
    int counter = 0;
    double acceleration = 0;
};


#endif //TRACKER_ACCELEROMETER_H
