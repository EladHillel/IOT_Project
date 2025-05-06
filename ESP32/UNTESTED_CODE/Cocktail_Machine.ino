#include "menu.h"
#include "motors_sensors.h"

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);
  
  setup_screen();
  setup_motors();
  setup_weight_sensor();
}

void loop() {
  CHECK_AND_HANDLE_TOUCH()
  if (order_pending){
    bool cup_placed = wait_for_cup();
    if (cup_placed){
      pour_drink(current_cocktail);
    }
    order_pending = false;
  }
  delay(200);
}

