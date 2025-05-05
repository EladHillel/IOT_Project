#include "cocktail_data.h"
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

TFT_eSPI tft = TFT_eSPI();

#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

const int SCREEN_W = 320;
const int SCREEN_H = 240;
const int SIDE_W = SCREEN_W - SCREEN_H;
const int MAIN_W = SCREEN_W - SIDE_W;
const int SIDE_BUTTON_H = SCREEN_H / 4;

int current_menu = 1;
int selected_cocktail = -1;
int touch_x = -1, touch_y = -1;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(1);

  tft.init();
  tft.setRotation(1);

  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);

  draw_UI();
}

void loop() {
  if (!touchscreen.touched())
    return;

  TS_Point p = touchscreen.getPoint();
  // truely voodoo
  touch_x = map(p.x, 200, 3700, 1, SCREEN_W);
  touch_y = map(p.y, 240, 3800, 1, SCREEN_H);

  handle_touch(touch_x, touch_y);
  delay(200);
}

void draw_UI() {
  tft.fillScreen(TFT_BLACK);
  draw_side_menu();
  draw_current_menu();
}

void draw_side_menu() {
  tft.setTextSize(2);
  const int th = 8 * 2;
  for (int i = 0; i < 4; i++) {
    int y = i * SIDE_BUTTON_H;
    tft.drawRect(MAIN_W, y, SIDE_W, SIDE_BUTTON_H, TFT_WHITE);
    String label = (i < 3) ? String(i + 1) : "Order";
    int16_t tw = tft.textWidth(label);
    int16_t tx = MAIN_W + (SIDE_W - tw) / 2;
    int16_t ty = y + (SIDE_BUTTON_H - th) / 2;
    tft.setCursor(tx, ty);
    tft.print(label);
  }
  tft.setTextSize(1);
}

void draw_menu_1() {
  const int lineH = 10;
  tft.setTextSize(1);
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      int x = i * (MAIN_W / 3);
      int y = j * (SCREEN_H / 3);
      int idx = j * 3 + i;
      tft.fillRect(x, y, MAIN_W / 3, SCREEN_H / 3, TFT_DARKGREY);
      tft.drawRect(x, y, MAIN_W / 3, SCREEN_H / 3, TFT_WHITE);
      tft.setCursor(x + 5, y + 6);
      tft.print(cocktails[idx].name);
      for (int k = 0; k < INGREDIENT_COUNT; k++) {
        tft.setCursor(x + 5, y + 10 + (k + 1) * lineH);
        tft.print(ingredients[k].name[0]);
        tft.print(": ");
        tft.print(cocktails[idx].amounts[k]);
        tft.print(" ml");
      }
      if (idx == selected_cocktail) {  // draw twice for thickness
        tft.drawRect(x + 2, y + 2, MAIN_W / 3 - 4, SCREEN_H / 3 - 4,
                     TFT_ORANGE);
        tft.drawRect(x + 3, y + 3, MAIN_W / 3 - 6, SCREEN_H / 3 - 6,
                     TFT_ORANGE);
      }
    }
  }
}

void draw_menu_2() {
  const int tileW = MAIN_W / 2;
  const int tileH = SCREEN_H / 2;
  const int button_size = 40;
  const int spacing = 10;

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
      int idx = j * 2 + i;
      int x = i * tileW;
      int y = j * tileH;
      tft.fillRect(x, y, tileW, tileH, ingredients[idx].color);
      tft.setTextSize(2);
      // name
      tft.setCursor(x + 8, y + 8);
      tft.print(ingredients[idx].name);
      // amount
      tft.setCursor(x + 8, y + 28);
      tft.print(ingredientAmounts[idx]);
      tft.print(" ml");
      // buttons lower in tile
      int button_total = button_size * 2 + spacing;
      int bx = x + (tileW - button_total) / 2;
      int by = y + tileH - button_size - 5;

      tft.setTextSize(3);

      tft.drawRect(bx, by, button_size, button_size, TFT_BLACK);
      tft.setCursor(bx + button_size / 2 - 8, by + button_size / 2 - 11);
      tft.print("+");

      tft.drawRect(bx + button_size + spacing, by, button_size,
                   button_size, TFT_BLACK);
      tft.setCursor(bx + button_size + spacing + button_size / 2 - 8,
                    by + button_size / 2 - 11);
      tft.print("-");
    }
  }
  tft.setTextSize(1);
}

void draw_menu_3() {
  tft.setTextSize(1);
  tft.setCursor(50, SCREEN_H / 2 - 5);
  tft.print("NOT IMPLEMENTED");
}

void draw_current_menu() {
  switch (current_menu) {
    case 1:
      draw_menu_1();
      break;
    case 2:
      draw_menu_2();
      break;
    case 3:
      draw_menu_3();
      break;
  }
}

void handle_touch(int x, int y) {
  if (x < 0 || y < 0)
    return;
  if (x >= MAIN_W) {
    int button = y / SIDE_BUTTON_H;
    if (button < 3) {
      current_menu = button + 1;
      draw_UI();
    } else {
      process_order();
    }
    return;
  }
  int tileW = MAIN_W / 2;
  int tileH = SCREEN_H / 2;

  const int lineH = 8 * 2;
  const int button_size = 40;
  const int spacing = 10;
  int button_total = button_size * 2 + spacing;
  int bx = (tileW - button_total) / 2;
  int by = tileH - button_size - 5;
  if (current_menu == 1) {
    selected_cocktail = (y / (SCREEN_H / 3)) * 3 + (x / (MAIN_W / 3));
    draw_menu_1();
  } else if (current_menu == 2) {
    int ci = x / tileW;
    int cj = y / tileH;
    int idx = cj * 2 + ci;
    int lx = x % tileW;
    int ly = y % tileH;
    if (lx >= bx && lx < bx + button_size && ly >= by && ly < by + button_size) {
      ingredientAmounts[idx] = min(200, ingredientAmounts[idx] + 25);
    } else if (lx >= bx + button_size + spacing && lx < bx + button_total && ly >= by && ly < by + button_size) {
      ingredientAmounts[idx] = max(0, ingredientAmounts[idx] - 25);
    }
    draw_menu_2();
  }
}

void process_order() {
  tft.fillScreen(TFT_BLACK);
  int y = 10;
  tft.setTextSize(2);
  tft.setCursor(10, y);
  tft.print("Order Details:");
  y += 20;
  if (current_menu == 1 && selected_cocktail >= 0) {
    tft.setCursor(10, y);
    tft.print(cocktails[selected_cocktail].name);
    y += 20;
    for (int k = 0; k < INGREDIENT_COUNT; k++) {
      tft.setCursor(10, y);
      tft.print(ingredients[k].name);
      tft.print(": ");
      tft.print(cocktails[selected_cocktail].amounts[k]);
      tft.print(" ml");
      y += 20;
    }
  } else if (current_menu == 2) {
    bool any = false;
    for (int i = 0; i < INGREDIENT_COUNT; i++) {
      if (ingredientAmounts[i] > 0) {
        tft.setCursor(10, y);
        tft.print(ingredients[i].name);
        tft.print(": ");
        tft.print(ingredientAmounts[i]);
        tft.print(" ml");
        y += 20;
        any = true;
      }
    }
    if (!any) {
      tft.setCursor(10, y);
      tft.print("No ingredients added!");
    }
  } else {
    tft.setCursor(10, y);
    tft.print("No cocktail selected!");
  }
  delay(3000);
  draw_UI();
}