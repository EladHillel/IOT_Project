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
  wait_for_cup();
  pour_drink(cocktails[8]);
  if (!touchscreen.touched())
    return;
  
  TS_Point p = touchscreen.getPoint();
  // truely voodoo
  touch_x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
  touch_y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
  handle_touch(touch_x, touch_y);
  delay(200);
}

