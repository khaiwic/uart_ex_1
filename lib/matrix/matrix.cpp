#include <Arduino.h>
#include "matrix.h"
#include "PinConfig.h"

button current_state;

void setupMatrix(){
    pinMode(MATRIX::analog_read, INPUT_PULLUP);
}
button scan(){
    uint16_t data = analogRead(MATRIX::analog_read);
    if (data >= 0 && data< 300) {
        current_state = button::TOP;
    } 
    else if (data >= 300 && data < 700) {
        current_state = button :: BACK;
    } 
    else if (data >= 700 && data < 1300) {
        current_state = button::LEFT; 
    }
    else if(data >= 1300 && data < 1800){
        current_state = button::RIGHT; 
    }
    else if(data >= 1800 && data < 2300){
        current_state = button::OK;
    }
    else if(data >= 2300 && data < 4095){
        current_state = button::NONE; // trang thai khong bam gi
    }
}