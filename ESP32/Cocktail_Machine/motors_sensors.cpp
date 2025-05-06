#include "motors_sensors.h"

void setup_motors(){
// Initialize motor control pins
  pinMode(MOTOR1_PIN, OUTPUT);
  pinMode(MOTOR2_PIN, OUTPUT);
  pinMode(MOTOR3_PIN, OUTPUT);
  
  // Start motors off
  digitalWrite(MOTOR1_PIN, LOW);
  digitalWrite(MOTOR2_PIN, LOW);
  digitalWrite(MOTOR3_PIN, LOW);
}

void setup_weight_sensor(){
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(CALIBRATION_FACTOR);

  Serial.println("Taring... remove any weight.");
  delay(3000);
  scale.tare();  // Zero the scale
  Serial.println("Tare complete.");

  while (!scale.is_ready()) {
    Serial.println("HX711 not found.");
    delay(1000);
  }
}

bool wait_for_cup(){
  while (scale.get_units(5) < CUP_WEIGHT_THRESHOLD) {
    Serial.print("Insert cup");
    Serial.println(scale.get_units(5));
    delay(100);
  }
  return true;
  Serial.print("CUP DETECTED");
  delay(1000);
}

void pour_drink(Cocktail cocktail){
  Serial.printf("Starting to pour cocktail: '%s'", cocktail.name);
  for(int ingredient = 0, ingredient < INGREDIENT_COUNT, ingredient++){
    pour_drink(ingredient, cocktail.amounts[ingredient])
  }
  Serial.printf("Cocktail poured successfully");
}

void pour_drink(int motor_num, float target_weight){
  Serial.printf("Starting motor number: %d for target weight: %.2f\n", motor_num, target_weight);
  float base_weight = scale.get_units(10);
  Serial.print("Base weight: %.2f\n", base_weight);

  digitalWrite(motor_num, HIGH);
  while (scale.get_units(5) < base_weight + target_weight) {
    Serial.print("Current overall weight: %.2f\n", scale.get_units(5));
    delay(100);
  }
  digitalWrite(motor_num, LOW);
  Serial.println("Target reached. Motor stopped.");
}

