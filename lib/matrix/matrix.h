#ifndef MATRIX_H
#define MATRIX_H
#include <Arduino.h>

enum class button{
    TOP,
    BACK,
    RIGHT,
    LEFT,
    OK,
    NONE
};

void setupMatrix();
button scan();

#endif