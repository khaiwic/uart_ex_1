#ifndef CONTROL_H
#define CONTROL_H

#include <Arduino.h>
#include "motor.h"
#include "PinConfig.h"
#include "matrix.h"

extern QueueHandle_t Ong_Truyen_Lenh;
extern volatile bool flag;
extern volatile bool flag_goal;
void PID_read();
void Encoder_read();
void Task_2(void *parameter);

#endif