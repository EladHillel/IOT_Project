#include "menu.h"
#include "motors_sensors.h"
#include "filesystem.h"
#include "bluetooth.h"

void setup() {
    Serial.begin(115200);
    while (!Serial)
        delay(10);

    // setup_motors();
    // setup_weight_sensor();
    setup_data();
    setup_screen();
    ble_setup();
    // enter_quick_mode(preset_cocktails[0]);
}

void loop() {
    ble_loop();
    check_and_handle_touch();
    if (order_pending) {
        if (isCocktailEmpty(ordered_cocktail)) {
            alert_error("Selected cocktail is empty.");
            order_pending = false;
            return;
        }
        Serial.print("Got valid order:");
        log_cocktail(ordered_cocktail);
        bool cup_placed = wait_for_cup();
        if (cup_placed) {
            pour_drink(ordered_cocktail, chosen_cocktail_size);
        }
        order_pending = false;
        update_top_ordered_cocktails();
    }
    delay(100);
}
