#include "HX711.h"

//MOTORs
const int MOTOR1_PIN = 12;
const int MOTOR2_PIN = 13;
const int MOTOR3_PIN = 14;


// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 4;
const int LOADCELL_SCK_PIN = 5;

// Replace this with your actual calibration factor
const float CALIBRATION_FACTOR = 93000/132;

HX711 scale;

void setup() {
  Serial.begin(57600);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(CALIBRATION_FACTOR);

  Serial.println("Taring... remove any weight.");
  delay(3000);
  scale.tare();  // Zero the scale
  Serial.println("Tare complete.");

  // Initialize motor control pins
  pinMode(MOTOR1_PIN, OUTPUT);
  pinMode(MOTOR2_PIN, OUTPUT);
  pinMode(MOTOR3_PIN, OUTPUT);
  
  // Start motors off
  digitalWrite(MOTOR1_PIN, LOW);
  digitalWrite(MOTOR2_PIN, LOW);
  digitalWrite(MOTOR3_PIN, LOW);
}

void run_motor_until_weight_reached(float target_weight, int motor_pin) {
  float base_weight = scale.get_units(10);
  Serial.print("Base: ");
  Serial.println(base_weight);

  digitalWrite(motor_pin, HIGH);
  while (scale.get_units(5) < base_weight + target_weight) {
    Serial.print("Current: ");
    Serial.println(scale.get_units(5));
    delay(100);
  }
  digitalWrite(motor_pin, LOW);
  Serial.println("Target reached. Motor stopped.");
  delay(1000); // Small pause between ingredients
}


void loop() {
  if (!scale.is_ready()) {
    Serial.println("HX711 not found.");
    delay(1000);
    return;
  }
  Serial.println("Starting:");
  delay(3000);
  run_motor_until_weight_reached(15, MOTOR1_PIN);
  for(;;);
}
