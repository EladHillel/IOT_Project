#include "menu.h"
#include "cocktail_data.h"

TFT_eSPI tft = TFT_eSPI();
SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

MenuState current_menu = Menu_1;
int menu_1_selected_cocktail_tile = -1; 
String current_cancellable_op_text = "";

void draw_menu_1_tile(int tile_index, bool selected) {
  const int lineH = 10;
  int i = tile_index % TABLE_DIMENSION;
  int j = tile_index / TABLE_DIMENSION;
  int x = i * (MAIN_WIDTH / TABLE_DIMENSION);
  int y = j * (SCREEN_HEIGHT / TABLE_DIMENSION);

  tft.fillRect(x, y, MAIN_WIDTH / 3, SCREEN_HEIGHT / 3, TFT_BLACK);
  tft.drawRect(x, y, MAIN_WIDTH / 3, SCREEN_HEIGHT / 3, TFT_WHITE);
  tft.setCursor(x + 5, y + 6);
  tft.print(preset_cocktails[tile_index].name);
  for (int k = 0; k < INGREDIENT_COUNT; k++) {
    tft.setCursor(x + 5, y + 10 + (k + 1) * lineH);
    tft.print(ingredients[k].name[0]);
    tft.print(": ");
    tft.print(preset_cocktails[tile_index].amounts[k]);
    tft.print(" ml");
  }

  if (selected) {
    tft.drawRect(x + 2, y + 2, MAIN_WIDTH / 3 - 4, SCREEN_HEIGHT / 3 - 4, TFT_ORANGE);
    tft.drawRect(x + 3, y + 3, MAIN_WIDTH / 3 - 6, SCREEN_HEIGHT / 3 - 6, TFT_ORANGE);
  }
}

void draw_menu_2_tile(int idx) {
  const int tileW = MAIN_WIDTH / 2;
  const int tileH = SCREEN_HEIGHT / 2;
  const int button_size = 40;
  const int spacing = 10;

  int i = idx % 2;
  int j = idx / 2;
  int x = i * tileW;
  int y = j * tileH;

  tft.fillRect(x, y, tileW, tileH, ingredients[idx].color);
  tft.setTextSize(2);

  // Name
  tft.setCursor(x + 8, y + 8);
  tft.print(ingredients[idx].name);

  // Amount
  tft.setCursor(x + 8, y + 28);
  tft.print(current_custom_cocktail.amounts[idx]);
  tft.print(" ml");

  // Buttons
  int button_total = button_size * 2 + spacing;
  int bx = x + (tileW - button_total) / 2;
  int by = y + tileH - button_size - 5;

  tft.setTextSize(3);

  // Plus
  tft.drawRect(bx, by, button_size, button_size, TFT_BLACK);
  tft.setCursor(bx + button_size / 2 - 8, by + button_size / 2 - 11);
  tft.print("+");

  // Minus
  tft.drawRect(bx + button_size + spacing, by, button_size, button_size, TFT_BLACK);
  tft.setCursor(bx + button_size + spacing + button_size / 2 - 8,
                by + button_size / 2 - 11);
  tft.print("-");
}

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
  tft.setTextSize(DEFAULT_TEXT_SIZE);
  for (int i = 0; i < TABLE_DIMENSION * TABLE_DIMENSION; i++) {
    bool is_selected = (preset_cocktails[i].name == current_preset_cocktail.name);
    draw_menu_1_tile(i, is_selected);
  }
}

void draw_menu_2() {
  tft.setTextSize(DEFAULT_TEXT_SIZE);
  for (int i = 0; i < 4; i++) {
    draw_menu_2_tile(i);
  }
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

void handle_touch_menu_1(int x, int y) {
  int new_tile = (y / (SCREEN_HEIGHT / 3)) * 3 + (x / (MAIN_WIDTH / 3));
  if (preset_cocktails[new_tile].name != current_preset_cocktail.name) {
    int prev_tile = -1;
    for (int i = 0; i < TABLE_DIMENSION * TABLE_DIMENSION; i++) {
      if (preset_cocktails[i].name == current_preset_cocktail.name) {
        prev_tile = i;
        break;
      }
    }

    current_preset_cocktail.name = preset_cocktails[new_tile].name;
    memcpy(current_preset_cocktail.amounts, preset_cocktails[new_tile].amounts, sizeof(current_preset_cocktail.amounts));
    ordered_cocktail = current_preset_cocktail;

    if (prev_tile != -1) draw_menu_1_tile(prev_tile, false);
    draw_menu_1_tile(new_tile, true);

    Serial.println("New cocktail chosen:");
    Serial.println(current_preset_cocktail.name);
  }
}

void handle_touch_menu_2(int x, int y) {
  const int tileW = MAIN_WIDTH / 2;
  const int tileH = SCREEN_HEIGHT / 2;
  const int button_size = 40;
  const int spacing = 10;
  const int button_total = button_size * 2 + spacing;

  int ci = x / tileW;
  int cj = y / tileH;
  int idx = cj * 2 + ci;

  int lx = x % tileW;
  int ly = y % tileH;

  int bx = (tileW - button_total) / 2;
  int by = tileH - button_size - 5;

  bool changed = false;

  if (lx >= bx && lx < bx + button_size && ly >= by && ly < by + button_size) {
    current_custom_cocktail.amounts[idx] = min(200, current_custom_cocktail.amounts[idx] + 25);
    changed = true;
  } else if (lx >= bx + button_size + spacing && lx < bx + button_total && ly >= by && ly < by + button_size) {
    current_custom_cocktail.amounts[idx] = max(0, current_custom_cocktail.amounts[idx] - 25);
    changed = true;
  }

  if (changed) {
    ordered_cocktail = current_custom_cocktail;
    draw_menu_2_tile(idx);
  }
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