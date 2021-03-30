#include "Accelerometer.h"

Accelerometer::Accelerometer() {
    Wire.begin();
    sensor = GY521(0x69);
}

void Accelerometer::begin() {
//    while (!sensor.wakeup()) {
//        Tasker::sleep(1000);
//    }
    sensor.setAccelSensitivity(0);
    sensor.setGyroSensitivity(0);
    sensor.setThrottle(false);

    sensor.axe = 0;
    sensor.aye = 0;
    sensor.aze = 0;
    sensor.gxe = 0;
    sensor.gye = 0;
    sensor.gze = 0;

}

void Accelerometer::read() {
    sensor.read();
    ax = sensor.getAccelX();
    ay = sensor.getAccelY();
    az = sensor.getAccelZ();
    DEBUG_PRINT("ACC: %f %f %f\n", ax, ay, az);
    acceleration = sqrt(square(ax) + square(ay) + square(az));
}

double Accelerometer::square(double a) {
    return a * a;
}

double Accelerometer::getAcceleration() const {
    return acceleration;
}
