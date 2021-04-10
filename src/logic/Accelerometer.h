#ifndef TRACKER_ACCELEROMETER_H
#define TRACKER_ACCELEROMETER_H

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "Constants.h"

class Accelerometer {
public:
    Accelerometer();

    static bool begin();

    static void
    read(double *acceleration, int16_t *accelerometer_x, int16_t *accelerometer_y, int16_t *accelerometer_z,
         int16_t *gyro_x, int16_t *gyro_y,
         int16_t *gyro_z, int16_t *temperature);

    double getAcceleration() const;

private:
    static double square(double a);

    Adafruit_MPU6050 mpu;
    float ax = 0, ay = 0, az = 0;
    double acceleration = 0;
};


#endif //TRACKER_ACCELEROMETER_H
