#include "matrix.h"
#include "PinConfig.h" 
#include <Arduino.h>

button Matrix_button[MATRIX::row][MATRIX::colo] = {
    {button::NONE, button::TOP, button::NONE},
    {button::LEFT, button::OK,  button::RIGHT},
    {button::NONE, button::BACK, button::NONE},
};

void setupMatrix(){
    for(int r = 0; r < MATRIX::row; r++){
        pinMode(MATRIX::rows[r], OUTPUT); 
        digitalWrite(MATRIX::rows[r], HIGH);
    }
    for(int c = 0; c < MATRIX::colo; c++){
        pinMode(MATRIX::colos[c], INPUT_PULLUP);
    }
}

button scan(){
    for(int r = 0; r < MATRIX::row; r++){
        digitalWrite(MATRIX::rows[r], LOW);
        for(int c = 0; c < MATRIX::colo; c++){
            if(digitalRead(MATRIX::colos[c]) == LOW){
   
                digitalWrite(MATRIX::rows[r], HIGH); 
                return Matrix_button[r][c];
            }
        }

        digitalWrite(MATRIX::rows[r], HIGH); 
    }
    return button::NONE;
}