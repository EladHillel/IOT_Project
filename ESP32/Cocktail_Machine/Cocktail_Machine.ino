#include "menu.h"
#include "motors_sensors.h"
#include "filesystem.h"

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);
  
  setup_motors();
  setup_weight_sensor();
  setup_data();
  setup_screen();
}


void loop() {
  check_and_handle_touch();
  if (order_pending){
    bool cup_placed = wait_for_cup();
    if (cup_placed){
      pour_drink(ordered_cocktail);
    }
    order_pending = false;
    return_to_main_menu();
  }
  delay(100);
}

