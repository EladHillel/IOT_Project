#ifndef MOTORS_SENSORS_H
#define MOTORS_SENSORS_H

#include "HX711.h"
#include "cocktail_data.h"

//MOTORs
const int MOTOR1_PIN = 22; // change
const int MOTOR2_PIN = 22; // change
const int MOTOR3_PIN = 22; // change
const int MOTOR4_PIN = 23; 

const int MOTOR_MAP[4] = {MOTOR1_PIN, MOTOR2_PIN, MOTOR3_PIN, MOTOR4_PIN};

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 5;
const int LOADCELL_SCK_PIN = 4;

const float CUP_WEIGHT_THRESHOLD = 0.5;

const float CALIBRATION_FACTOR = 93000/132;

static HX711 scale;

/*
Setups motor functionality
*/
void setup_motors();

/*
Setups weight sensor functionality
*/
void setup_weight_sensor();

/*
Waits for cup insertion, returns false if operation cancelled. else true.
*/
bool wait_for_cup();

/*
Pours the cocktail.
*/
void pour_drink(Cocktail cocktail);

#endif