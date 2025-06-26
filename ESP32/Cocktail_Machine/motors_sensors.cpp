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

void setup_weight_sensor() {
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(CALIBRATION_FACTOR);

  Serial.println("Checking if HX711 is ready...");
  while (!scale.is_ready()) {
    Serial.println("HX711 not found.");
    delay(1000);
  }
  Serial.println("HX711 found.");

  Serial.println("Taring... remove any weight.");
  delay(3000);
  scale.tare();  // Zero the scale
  Serial.println("Tare complete.");
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

void pour_drink(Cocktail cocktail, CocktailSize size) {
  init_cancellable_op("Pouring cocktail...");
  delay(500);
  Serial.printf("Starting to pour cocktail: '%s'\n", cocktail.name.c_str());
  Serial.printf("Cocktail amount modified by: '%.3f'\n", PORTION_MAP[size]);
  for(int ingredient = 0; ingredient < INGREDIENT_COUNT; ingredient++){
    if (cocktail.amounts[ingredient] == 0 ){
      continue;
    }
    
    float curr_amount = cocktail.amounts[ingredient] * PORTION_MAP[size];
    OrderState op_state = pour_ingredient(ingredient, curr_amount);
    switch (op_state) {
    case Completed:
      Serial.print("Ingredient poured succcessfully");
      break;
    case Cancelled:
      Serial.println("CANCELLED");
      update_stats_on_drink_order(cocktail, op_state);
      return_to_main_menu();
      return;
    case Timeout:
      Serial.println("Timeout Reached");
      update_stats_on_drink_order(cocktail, op_state);
      alert_error("Operation failed: pour timeout reached");
      return;
    }
  }
  update_stats_on_drink_order(cocktail, Completed);
  Serial.printf("Cocktail poured successfully");
  return_to_main_menu();
}

static OrderState pour_ingredient(int motor_num, float target_weight){ 

  //Logging base weight & starting motor
  Serial.printf("Starting motor number: %d for target weight: %.2f\n", motor_num, target_weight);
  float base_weight = scale.get_units(10);
  Serial.printf("Base weight: %.2f\n", base_weight);
  digitalWrite(MOTOR_MAP[motor_num], HIGH);


  float prev_weight = base_weight;
  int times_unchanged = 0;
  float curr_weight = scale.get_units(5);
  //While target not reached
  while (curr_weight < base_weight + target_weight) {
    Serial.printf("Current overall weight: %.2f\n", scale.get_units(5));

    //Check if cancelled
    check_and_handle_touch();
    if (current_menu != Cancellable_Op){
      digitalWrite(MOTOR_MAP[motor_num], LOW);
      update_ingredient_amount(motor_num, curr_weight - base_weight);
      Serial.println("Cancelled in pour_ingredient");
      return Cancelled;
    }

    //Timeout check
    bool is_weight_changed = (abs(curr_weight - prev_weight) < WEIGHT_CHANGE_DETECTION_THRESHOLD);
    Serial.printf("Weight change check: prev=%.2f, curr=%.2f, times_unchanged=%d\n", prev_weight, curr_weight, times_unchanged);
    times_unchanged =  is_weight_changed ? times_unchanged + 1 : 0;
    if (times_unchanged >= TIMES_UNCHANGED_FOR_TIMEOUT) {
      digitalWrite(MOTOR_MAP[motor_num], LOW);
      update_ingredient_amount(motor_num, curr_weight - base_weight);
      return Timeout;
    }
    
    prev_weight = curr_weight;//is_weight_changed ? curr_weight : prev_weight;
    delay(100);
    curr_weight = scale.get_units(5);
  }

  //target reached
  digitalWrite(MOTOR_MAP[motor_num], LOW);
  Serial.println("Target reached. Motor stopped.");
  update_ingredient_amount(motor_num, scale.get_units(5) - base_weight);
  return Completed;
}


void pour_until_stopped(int motor_num){
  if(motor_num == -1){
    Serial.println("pour_until_stopped index not valid");
    return;
  }
  Serial.println("Starting Cleaning Mode...");
  init_cancellable_op("Cleaning...");
  const int CLEAN_TIME = 5000;
  unsigned long lastSentTime = millis();
  unsigned long currentTime = millis();
  
  digitalWrite(MOTOR_MAP[motor_num], HIGH);

  while (currentTime - lastSentTime < CLEAN_TIME) {
    //Check if cancelled
    check_and_handle_touch();
    if (current_menu != Cancellable_Op){
      digitalWrite(MOTOR_MAP[motor_num], LOW);
      Serial.println("Cancelled cleanup");
      return;
    }
    currentTime = millis();
    delay(100);
  }
  Serial.println("Cleanup completed");
  digitalWrite(MOTOR_MAP[motor_num], LOW);
  return_to_main_menu();
}
