#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <Arduino.h>
#include "cocktail_data.h"  

void ble_setup();
void ble_loop();

void log_cocktail(const Cocktail& cocktail);
void send_menu_via_ble();
void send_stats_via_ble();
void send_push_notification(int ingredientIndex);
#endif 
