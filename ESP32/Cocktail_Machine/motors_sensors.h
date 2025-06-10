#ifndef MOTORS_SENSORS_H
#define MOTORS_SENSORS_H

#include "HX711.h"
#include "cocktail_data.h"
#include "menu.h"

//MOTORs
const int MOTOR1_PIN = 18; // change
const int MOTOR2_PIN = 19; // change
const int MOTOR3_PIN = 22; // change
const int MOTOR4_PIN = 23; 
const int MOTOR_MAP[4] = {MOTOR1_PIN, MOTOR2_PIN, MOTOR3_PIN, MOTOR4_PIN};
const float PORTION_MAP[3] = {0.75, 1, 1.25};
// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 5;
const int LOADCELL_SCK_PIN = 4;

const float CUP_WEIGHT_THRESHOLD = 1.2;
const float BASE_WEIGHT_POSSIBLE_ERROR = 2;
const float TIMES_UNCHANGED_FOR_TIMEOUT = 45;
const float WEIGHT_CHANGE_DETECTION_THRESHOLD = 1.5;

const float CALIBRATION_FACTOR = 93000/132;

static HX711 scale;

void setup_motors();

void setup_weight_sensor();

bool wait_for_cup();

void pour_drink(Cocktail cocktail, CocktailSize size);

static OrderState pour_ingredient(int motor_num, float weight);

#endif