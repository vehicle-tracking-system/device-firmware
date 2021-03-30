#ifndef UNTITLED_ACCELEROMETER_H
#define UNTITLED_ACCELEROMETER_H

#include <Wire.h>
#include "GY521.h"
#include "Constants.h"

class Accelerometer {
public:
    Accelerometer();

    void begin();

    void read();

    double getAcceleration() const;

private:
    static double square(double a);

    GY521 sensor;
    float ax = 0, ay = 0, az = 0;
    double acceleration = 0;
};


#endif //UNTITLED_ACCELEROMETER_H
