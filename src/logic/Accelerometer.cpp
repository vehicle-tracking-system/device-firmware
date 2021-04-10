#include "Accelerometer.h"

Accelerometer::Accelerometer() {

}

bool Accelerometer::begin() {
    Wire.begin();
    Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
    Wire.write(MPU_ADDR); // PWR_MGMT_1 register
    Wire.write(0); // set to zero (wakes up the MPU-6050)
    Wire.endTransmission(true);
    return true;
}

void
Accelerometer::read(double *acceleration, int16_t *accelerometer_x, int16_t *accelerometer_y, int16_t *accelerometer_z,
                    int16_t *gyro_x, int16_t *gyro_y, int16_t *gyro_z, int16_t *temperature) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, 7 * 2, true);
    int16_t a, b, c;
    a = Wire.read() << 8 | Wire.read(); // reading registers: 0x3B (ACCEL_XOUT_H) and 0x3C (ACCEL_XOUT_L)
    b = Wire.read() << 8 | Wire.read(); // reading registers: 0x3D (ACCEL_YOUT_H) and 0x3E (ACCEL_YOUT_L)
    c = Wire.read() << 8 | Wire.read(); // reading registers: 0x3F (ACCEL_ZOUT_H) and 0x40 (ACCEL_ZOUT_L)
    *temperature = Wire.read() << 8 | Wire.read(); // reading registers: 0x41 (TEMP_OUT_H) and 0x42 (TEMP_OUT_L)
    *gyro_x = Wire.read() << 8 | Wire.read(); // reading registers: 0x43 (GYRO_XOUT_H) and 0x44 (GYRO_XOUT_L)
    *gyro_y = Wire.read() << 8 | Wire.read(); // reading registers: 0x45 (GYRO_YOUT_H) and 0x46 (GYRO_YOUT_L)
    *gyro_z = Wire.read() << 8 | Wire.read(); // reading registers: 0x47 (GYRO_ZOUT_H) and 0x48 (GYRO_ZOUT_L)
    *accelerometer_x = a;
    *accelerometer_y = b;
    *accelerometer_z = c;
    *acceleration = sqrt(square(a) + square(b) + square(c));
}

double Accelerometer::square(double a) {
    return a * a;
}

double Accelerometer::getAcceleration() const {
    return acceleration;
}
