#include <Arduino.h>
#include "encoderConfig.h"
#include "PinConfig.h"
#include "motor.h"

volatile int encoderA_values = 0;
volatile int encoderB_values = 0;

//setupMotor
void initMotor(){  
    //MotorA
    pinMode(MOTOR::ina_1, OUTPUT);
    pinMode(MOTOR::ina_2, OUTPUT);
    pinMode(MOTOR::pwma, OUTPUT);
    ledcSetup(0, MOTOR::freq, MOTOR::resol); 
    ledcAttachPin(MOTOR::pwma, 0);
    
    //MotorB
    pinMode(MOTOR::inb_1, OUTPUT);
    pinMode(MOTOR::inb_2, OUTPUT);
    pinMode(MOTOR::pwmb, OUTPUT);
    ledcSetup(1, MOTOR::freq, MOTOR::resol); 
    ledcAttachPin(MOTOR::pwmb, 1);          
}
//setupEncoder
void initEncoder(){
    pinMode(ENCODER::encoder_1A, INPUT_PULLUP);
    pinMode(ENCODER::encoder_1B, INPUT_PULLUP);
    pinMode(ENCODER::encoder_2A, INPUT_PULLUP);
    pinMode(ENCODER::encoder_2B, INPUT_PULLUP);
    
    attachInterrupt(digitalPinToInterrupt(ENCODER::encoder_1A), positionA, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER::encoder_2A), positionB, CHANGE);
}
//interrupt encoder A
void IRAM_ATTR positionA(){
    int A = digitalRead(ENCODER::encoder_1A);
    int B = digitalRead(ENCODER::encoder_1B);
    if((A == HIGH) != (B == LOW)){
        encoderA_values --;
    }
    else{
        encoderA_values ++;
    }
}
//interrupt encoder B
void IRAM_ATTR positionB(){
    int A = digitalRead(ENCODER::encoder_2A);
    int B = digitalRead(ENCODER::encoder_2B);
    if((A == HIGH) != (B == LOW)){
        encoderB_values ++;
    }
    else{
        encoderB_values --;
    }
}
//reset
void reset(){
    encoderA_values = 0;
    encoderB_values = 0;
}
//receive speed
void go(control next, int speedA, int speedB) {
    if (next == control::STOP) {
        digitalWrite(MOTOR::ina_1, LOW); digitalWrite(MOTOR::ina_2, LOW);
        digitalWrite(MOTOR::inb_1, LOW); digitalWrite(MOTOR::inb_2, LOW);
        ledcWrite(MOTOR::pwma, 0);
        ledcWrite(MOTOR::pwmb, 0);
        return; // Thoát hàm luôn
    }

    if (speedA > 0) {
        digitalWrite(MOTOR::ina_1, HIGH); digitalWrite(MOTOR::ina_2, LOW); 
    } 
    else if (speedA < 0) {
        digitalWrite(MOTOR::ina_1, LOW); digitalWrite(MOTOR::ina_2, HIGH); 
    } 
    else {
        digitalWrite(MOTOR::ina_1, LOW); digitalWrite(MOTOR::ina_2, LOW);
    }

    int pwm_A = abs(speedA);
    if (pwm_A > 1023) pwm_A = 1023; 
    ledcWrite(MOTOR::pwma, pwm_A);

    if (speedB > 0) {
        digitalWrite(MOTOR::inb_1, HIGH); digitalWrite(MOTOR::inb_2, LOW); 
    } 
    else if (speedB < 0) {
        digitalWrite(MOTOR::inb_1, LOW); digitalWrite(MOTOR::inb_2, HIGH); 
    } 
    else {
        digitalWrite(MOTOR::inb_1, LOW); digitalWrite(MOTOR::inb_2, LOW);  
    }
    
    int pwm_B = abs(speedB);
    if (pwm_B > 1023) pwm_B = 1023; 
    ledcWrite(MOTOR::pwmb, pwm_B);
}