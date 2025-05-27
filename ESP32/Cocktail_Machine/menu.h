#ifndef MENU_H
#define MENU_H

#include "cocktail_data.h"
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

extern TFT_eSPI tft;
extern SPIClass touchscreenSPI;
extern XPT2046_Touchscreen touchscreen;

static const int SCREEN_WIDTH = 320;
static const int SCREEN_HEIGHT = 240;
static const int SIDE_WIDTH = SCREEN_WIDTH - SCREEN_HEIGHT;
static const int MAIN_WIDTH = SCREEN_WIDTH - SIDE_WIDTH;
static const int SIDE_BUTTON_HEIGHT = SCREEN_HEIGHT / 4;
static const int SIDE_MENU_TEXT_SIZE = 2;
static const int SIDE_MENU_RECT_AMOUNT = 4;
static const int DEFAULT_TEXT_SIZE = 1;
static const int TABLE_DIMENSION = 3;
static const int CANCELLABLE_OP_TEXT_SIZE = 3;
static const int CANCEL_BUTTON_SIZE = 40;
static const int CANCEL_MENU_TEXT_CENTER_X = SCREEN_WIDTH / 2;
static const int CANCEL_MENU_TEXT_CENTER_Y = SCREEN_HEIGHT / 3;
static const int CANCEL_BUTTON_X = CANCEL_MENU_TEXT_CENTER_X - CANCEL_BUTTON_SIZE / 2;
static const int CANCEL_BUTTON_Y = CANCEL_MENU_TEXT_CENTER_Y + 40;
static const int MENU_2_INGREDIENT_DELTA = 10;
static const int LONG_PRESS_INTERVAL_MS = 10;
static const int LONG_PRESS_COUNT_THRESHOLD = 500;


enum MenuState {
  Menu_1 = 1,
  Menu_2 = 2,
  Menu_3 = 3,
  Cancellable_Op,
  Error_Screen,
  Cocktail_More,
};

extern MenuState current_menu;
extern int menu_1_selected_cocktail_tile;

/*
Performs setup steps for screen
*/
void setup_screen();

/*
returns to menu 1
*/
void return_to_main_menu();

/*
Main function for drawing the UI in the machines screen.
*/
void draw_current_menu();

/*
Checks for and handles user input
*/
void check_and_handle_touch();

/*
initiates cancellable operation
*/
void init_cancellable_op(String op_text);

/*
Shows error message
*/
void alert_error(String msg);

/*
Shows more cocktail options and data
*/
void open_cocktail_more();


void handle_touch(int x, int y);
TS_Point* check_touch();
void draw_side_menu();
void draw_menu_1();
void draw_menu_2();
void draw_menu_3();
void draw_cancellable_operation();
void handle_touch_side_menu(int x, int y);
void handle_touch_cancellable_op(int x, int y);
void handle_touch_menu_1(int x, int y);
void handle_touch_menu_2(int x, int y);
void handle_touch_menu_3(int x, int y);
void draw_error_screen();
void draw_cocktail_more_menu();
void handle_touch_cocktail_extended_menu(int x, int y);

#endif