#include "menu.h"
#include "cocktail_data.h"

TFT_eSPI tft = TFT_eSPI();
SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

MenuState current_menu = Menu_1;
int menu_1_selected_cocktail_tile = -1; 
String current_cancellable_op_text = "";

void setup_screen(){
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(1);

  tft.init();
  tft.setRotation(1);

  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);

  draw_current_menu();
}

void draw_side_menu() {
  tft.setTextSize(SIDE_MENU_TEXT_SIZE);
  const int text_height_in_pixels = 8 * SIDE_MENU_TEXT_SIZE;
  
  for (int i = 0; i < SIDE_MENU_RECT_AMOUNT; i++) {
    int current_button_y = i * SIDE_BUTTON_HEIGHT;
    tft.drawRect(MAIN_WIDTH, current_button_y, SIDE_WIDTH, SIDE_BUTTON_HEIGHT, TFT_WHITE);
	
    String current_button_label = (i < 3) ? String(i + 1) : "Order";
    int16_t text_width = tft.textWidth(current_button_label);
    int16_t text_pos_x = MAIN_WIDTH + (SIDE_WIDTH - text_width) / 2;
    int16_t text_pos_y = current_button_y + (SIDE_BUTTON_HEIGHT - text_height_in_pixels) / 2;
	
    tft.setCursor(text_pos_x, text_pos_y);
    tft.print(current_button_label);
  }
  tft.setTextSize(DEFAULT_TEXT_SIZE);
}

void draw_menu_1() {
  const int lineH = 10;
  tft.setTextSize(DEFAULT_TEXT_SIZE);
  
  for (int i = 0; i < TABLE_DIMENSION; i++) {
    for (int j = 0; j < TABLE_DIMENSION; j++) {
      int current_button_x = i * (MAIN_WIDTH / TABLE_DIMENSION);
      int current_button_y = j * (SCREEN_HEIGHT / TABLE_DIMENSION);
      int cell_num = j * TABLE_DIMENSION + i;
      tft.fillRect(current_button_x, current_button_y, MAIN_WIDTH / 3, SCREEN_HEIGHT / 3, TFT_BLACK);
      tft.drawRect(current_button_x, current_button_y, MAIN_WIDTH / 3, SCREEN_HEIGHT / 3, TFT_WHITE);
      tft.setCursor(current_button_x + 5, current_button_y + 6);
      tft.print(cocktails[cell_num].name);
      for (int k = 0; k < INGREDIENT_COUNT; k++) {
        tft.setCursor(current_button_x + 5, current_button_y + 10 + (k + 1) * lineH);
        tft.print(ingredients[k].name[0]);
        tft.print(": ");
        tft.print(cocktails[cell_num].amounts[k]);
        tft.print(" ml");
      }
      if (cell_num == menu_1_selected_cocktail_tile) {  // draw twice for thickness
        tft.drawRect(current_button_x + 2, current_button_y + 2, MAIN_WIDTH / 3 - 4, SCREEN_HEIGHT / 3 - 4,
                     TFT_ORANGE);
        tft.drawRect(current_button_x + 3, current_button_y + 3, MAIN_WIDTH / 3 - 6, SCREEN_HEIGHT / 3 - 6,
                     TFT_ORANGE);
      }
    }
  }
}

void draw_menu_2() { //need cleanup
  const int tileW = MAIN_WIDTH / 2;
  const int tileH = SCREEN_HEIGHT / 2;
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
      tft.print(current_custom_cocktail.amounts[idx]);
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
  tft.setTextSize(DEFAULT_TEXT_SIZE);
}

void draw_menu_3() {
  tft.setTextSize(DEFAULT_TEXT_SIZE);
  tft.setCursor(50, SCREEN_HEIGHT / 2 - 5);
  tft.print("NOT IMPLEMENTED");
}

void draw_current_menu() {
  Serial.println("Drawing current menu:");
  Serial.println(current_menu);
  tft.fillScreen(TFT_BLACK);
  switch (current_menu) {
    case Menu_1:
      draw_side_menu();
      draw_menu_1();
      break;
    case Menu_2:
      draw_side_menu();
      draw_menu_2();
      break;
    case Menu_3:
      draw_side_menu();
      draw_menu_3();
      break;
    case Cancellable_Op:
      draw_cancellable_operation();
      break;
  }
}

void draw_cancellable_operation(){
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM); // Middle center
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(DEFAULT_TEXT_SIZE);

  tft.drawString(current_cancellable_op_text, CANCEL_MENU_TEXT_CENTER_X, CANCEL_MENU_TEXT_CENTER_Y);

  
  tft.fillRect(CANCEL_BUTTON_X, CANCEL_BUTTON_Y, CANCEL_BUTTON_SIZE, CANCEL_BUTTON_SIZE, TFT_RED);
  tft.setTextColor(TFT_WHITE, TFT_RED);
  tft.setTextSize(DEFAULT_TEXT_SIZE);
  tft.drawString("X", CANCEL_MENU_TEXT_CENTER_X, CANCEL_BUTTON_Y + CANCEL_BUTTON_SIZE / 2);

  // "Cancel" label
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Cancel", CANCEL_MENU_TEXT_CENTER_X, CANCEL_BUTTON_Y + CANCEL_BUTTON_SIZE + 10);
}

TS_Point* check_touch(){
  static TS_Point point_touched;

  if (!touchscreen.touched())
    return nullptr;

  point_touched = touchscreen.getPoint();
  return &point_touched;
}

void handle_touch(TS_Point point) {
  int x = map(point.x, 200, 3700, 1, SCREEN_WIDTH);
  int y = map(point.y, 240, 3800, 1, SCREEN_HEIGHT);
  if (x < 0 || y < 0)
    return;

  if (current_menu == Cancellable_Op) {
    handle_touch_cancellable_op(x, y);
    return;
  }

  if (x >= MAIN_WIDTH) {
    handle_touch_side_menu(x, y);
    return;
  } else {
    switch (current_menu) {
      case Menu_1:
        handle_touch_menu_1(x, y);
        break;
      case Menu_2:
        handle_touch_menu_2(x, y);
        break;
      case Menu_3:
        handle_touch_menu_3(x, y);
        break;
      default:
        return;
    }
  }
  draw_current_menu();
}

void handle_touch_side_menu(int x, int y){
  int button = y / SIDE_BUTTON_HEIGHT;
    if (button < 3) {
      current_menu = static_cast<MenuState>(button + 1);
      draw_current_menu();
    } 
    else {
      order_pending = true;
    }
}

void handle_touch_cancellable_op(int x, int y){
  if (x >= CANCEL_BUTTON_X && x <= CANCEL_BUTTON_X + CANCEL_BUTTON_SIZE &&
              y >= CANCEL_BUTTON_Y && y <= CANCEL_BUTTON_Y + CANCEL_BUTTON_SIZE)
    {
      current_menu = Menu_1; //Cancellable ops must poll for menu state.
    }
  draw_current_menu();
}

void handle_touch_menu_1(int x, int y){
  menu_1_selected_cocktail_tile = (y / (SCREEN_HEIGHT / 3)) * 3 + (x / (MAIN_WIDTH / 3));
  current_preset_cocktail.name = cocktails[menu_1_selected_cocktail_tile].name;
  memcpy(current_preset_cocktail.amounts, cocktails[menu_1_selected_cocktail_tile].amounts, sizeof(current_preset_cocktail.amounts));
  Serial.println("New cocktail chosen:");
  Serial.println(current_preset_cocktail.name);
  ordered_cocktail = current_preset_cocktail;
  draw_current_menu();
}

void handle_touch_menu_2(int x, int y){ //need cleanup
  int tileW = MAIN_WIDTH / 2;
  int tileH = SCREEN_HEIGHT / 2;
  const int button_size = 40;
  const int spacing = 10;
  int button_total = button_size * 2 + spacing;
  int bx = (tileW - button_total) / 2;
  int by = tileH - button_size - 5;

  int ci = x / tileW;
  int cj = y / tileH;
  int idx = cj * 2 + ci;
  int lx = x % tileW;
  int ly = y % tileH;
  if (lx >= bx && lx < bx + button_size && ly >= by && ly < by + button_size) {
    current_custom_cocktail.amounts[idx] = min(200, current_custom_cocktail.amounts[idx] + 25);
  } else if (lx >= bx + button_size + spacing && lx < bx + button_total && ly >= by && ly < by + button_size) {
    current_custom_cocktail.amounts[idx] = max(0, current_custom_cocktail.amounts[idx] - 25);
  }
  ordered_cocktail = current_custom_cocktail;
}

void handle_touch_menu_3(int x, int y){
  return;
}

void init_cancellable_op(String op_text){
  current_menu = Cancellable_Op;
  current_cancellable_op_text = op_text;
  draw_current_menu();
}

void check_and_handle_touch() {
  TS_Point* p = check_touch();
  if (p){
    handle_touch(*p);
  }
}

void return_to_main_menu(){
  current_menu = Menu_1;
  draw_current_menu();
}