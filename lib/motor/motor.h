#ifndef MOTOR_H
#define MOTOR_H
#include <Arduino.h>
#include "PinConfig.h"
#include "encoderConfig.h"
extern volatile int encoderA_values;
extern volatile int encoderB_values;
enum class control{
    TOP,
    LEFT,
    RIGHT, 
    BACK,
    STOP,
    FINISH,
};
//setup motor
void initMotor();
void go(control next, int speedA, int speedB);
//setup control by encoder
void initEncoder();
void positionA();
void positionB();

void reset();
#endif