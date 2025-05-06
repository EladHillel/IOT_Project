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

extern int current_menu;
extern int selected_cocktail;
extern int touch_x;
extern int touch_y;


/*
Performs setup steps for screen
*/
void setup_screen();

/*
Main function for drawing the UI in the machines screen.
*/
void draw_UI();

/*
Draws the side menu, which allow navigation between menus.
*/
void draw_side_menu();

/*
Handles user input.
*/
void handle_touch(int x, int y);

/*
draws the current selected menu.
*/
void draw_current_menu();

/*
Draws menu 1 - menu with preselected cocktails
*/
static void draw_menu_1();

/*
Draws menu 1 - menu for creating custom cocktails
*/
static void draw_menu_2();

/*
Draws menu 3 - menu for other utils.
*/
static void draw_menu_3();

#endif