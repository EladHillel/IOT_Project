#include "motors_sensors.h"
#include "menu.h"

void setup_motors(){
// Initialize motor control pins
  pinMode(MOTOR1_PIN, OUTPUT);
  pinMode(MOTOR2_PIN, OUTPUT);
  pinMode(MOTOR3_PIN, OUTPUT);
  pinMode(MOTOR4_PIN, OUTPUT);
  
  // Start motors off
  digitalWrite(MOTOR1_PIN, LOW);
  digitalWrite(MOTOR2_PIN, LOW);
  digitalWrite(MOTOR3_PIN, LOW);
  digitalWrite(MOTOR4_PIN, LOW);
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
  Serial.println("HX711 found.");
}

bool wait_for_cup() {
  const int stable_reads_required = 10;
  const int delay_ms = 100;
  const float delta_threshold = CUP_WEIGHT_THRESHOLD;
  int stable_reads = 0;

  init_cancellable_op("Please insert a cup.");
  float baseline = scale.get_units(10);  // do NOT tare, just read average

  while (stable_reads < stable_reads_required) {
    check_and_handle_touch();
    if (current_menu != Cancellable_Op) {
      Serial.println("CANCELLED");
      return false;
    }

    float weight = scale.get_units(10);
    float delta = weight - baseline;

    Serial.print("Insert cup, Δ: ");
    Serial.println(delta);

    if (delta >= delta_threshold) {
      stable_reads++;
    } else {
      stable_reads = 0;
    }

    delay(delay_ms);
  }

  Serial.println("CUP DETECTED");
  return true;
}

void pour_drink(Cocktail cocktail){
  init_cancellable_op("Pouring cocktail...");
  delay(500);
  Serial.printf("Starting to pour cocktail: '%s'\n", cocktail.name);
  for(int ingredient = 0; ingredient < INGREDIENT_COUNT; ingredient++){
    if (cocktail.amounts[ingredient] == 0 ){
      continue;
    }
    
    bool op_cancelled = pour_ingredient(ingredient, cocktail.amounts[ingredient]);

    if (op_cancelled){
      Serial.print("CANCELLED");
      return;
    }
  }
  Serial.printf("Cocktail poured successfully");
}

static bool pour_ingredient(int motor_num, float target_weight){ //returns whether op was cancelled
  current_menu = Cancellable_Op;
  Serial.printf("Starting motor number: %d for target weight: %.2f\n", motor_num, target_weight);
  float base_weight = scale.get_units(10);
  Serial.printf("Base weight: %.2f\n", base_weight);

  digitalWrite(MOTOR_MAP[motor_num], HIGH);
  while (scale.get_units(5) < base_weight + target_weight) {
    check_and_handle_touch();
    if (current_menu != Cancellable_Op){
      digitalWrite(MOTOR_MAP[motor_num], LOW);
      return true;
    }
    Serial.printf("Current overall weight: %.2f\n", scale.get_units(5));
    delay(100);
  }
  digitalWrite(MOTOR_MAP[motor_num], LOW);
  Serial.println("Target reached. Motor stopped.");
  return false;
}

