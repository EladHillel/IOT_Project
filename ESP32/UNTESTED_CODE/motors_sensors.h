#ifndef MOTORS_SENSORS_H
#define MOTORS_SENSORS_H

#include "HX711.h"
#include "cocktail_data.h"

//MOTORs
const int MOTOR1_PIN = 22; // change
const int MOTOR2_PIN = 22; // change
const int MOTOR3_PIN = 22; // change
const int MOTOR4_PIN = 23; 

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 5;
const int LOADCELL_SCK_PIN = 4;

const float CUP_WEIGHT_THRESHOLD = 1.2;

const float CALIBRATION_FACTOR = 93000/132;

static HX711 scale;

void setup_motors();

void setup_weight_sensor();

bool wait_for_cup();

void pour_drink(Cocktail cocktail);

static void pour_ingredient(int motor_num, float weight);

#endif