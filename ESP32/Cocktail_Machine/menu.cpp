#include "menu.h"
#include "cocktail_data.h"

TFT_eSPI tft = TFT_eSPI();
SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

MenuState current_menu = Menu_1;
int menu_1_selected_cocktail_tile = -1; 
String current_cancellable_op_text = "";
String current_error_message = "";

void draw_menu_1_tile(int tile_index, bool selected, bool cocktail_available) {
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

  if (!cocktail_available) {
    tft.drawLine(x, y, x + MAIN_WIDTH / 3, y + SCREEN_HEIGHT / 3, TFT_RED);
    tft.drawLine(x + MAIN_WIDTH / 3, y, x, y + SCREEN_HEIGHT / 3, TFT_RED);
    return;
  }

  if (selected) {
    tft.drawRect(x + 2, y + 2, MAIN_WIDTH / 3 - 4, SCREEN_HEIGHT / 3 - 4, TFT_ORANGE);
    tft.drawRect(x + 3, y + 3, MAIN_WIDTH / 3 - 6, SCREEN_HEIGHT / 3 - 6, TFT_ORANGE);
  }
}

void draw_menu_2_tile(int ingredient_index, bool can_add) {
  const int tileW = MAIN_WIDTH / 2;
  const int tileH = SCREEN_HEIGHT / 2;
  const int button_size = 40;
  const int spacing = 10;

  int i = ingredient_index % 2;
  int j = ingredient_index / 2;
  int x = i * tileW;
  int y = j * tileH;

  tft.fillRect(x, y, tileW, tileH, ingredients[ingredient_index].color);
  tft.setTextSize(2);

  // Name
  tft.setCursor(x + 8, y + 8);
  tft.setTextColor(TFT_BLACK, ingredients[ingredient_index].color);
  tft.print(ingredients[ingredient_index].name);

  // Amount
  tft.setCursor(x + 8, y + 28);
  tft.print(current_custom_cocktail.amounts[ingredient_index]);
  tft.print(" ml");

  // Buttons
int button_total = button_size * 2 + spacing;
int bx = x + (tileW - button_total) / 2;
int by = y + tileH - button_size - 5;

tft.setTextSize(3);

// Draw + button
tft.drawRect(bx, by, button_size, button_size, TFT_BLACK);
tft.setTextColor(TFT_BLACK, ingredients[ingredient_index].color);
tft.setCursor(bx + button_size / 2 - 8, by + button_size / 2 - 11);
tft.print("+");

// If unavailable, overlay red X
if (!can_add) {
  tft.drawLine(bx, by, bx + button_size, by + button_size, TFT_RED);
  tft.drawLine(bx + button_size, by, bx, by + button_size, TFT_RED);
}

// Draw -
tft.drawRect(bx + button_size + spacing, by, button_size, button_size, TFT_BLACK);
tft.setTextColor(TFT_BLACK, ingredients[ingredient_index].color);
tft.setCursor(bx + button_size + spacing + button_size / 2 - 8, by + button_size / 2 - 11);
tft.print("-");

// If subtract is unavailable, overlay red X
if (current_custom_cocktail.amounts[ingredient_index] == 0) {
  int sub_bx = bx + button_size + spacing;
  tft.drawLine(sub_bx, by, sub_bx + button_size, by + button_size, TFT_RED);
  tft.drawLine(sub_bx + button_size, by, sub_bx, by + button_size, TFT_RED);
}

// Reset text color globally for other menus
tft.setTextColor(TFT_WHITE, TFT_BLACK);
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
  ordered_cocktail = current_preset_cocktail;
  tft.setTextSize(DEFAULT_TEXT_SIZE);
  for (int i = 0; i < TABLE_DIMENSION * TABLE_DIMENSION; i++) {
    bool is_selected = (preset_cocktails[i].name == current_preset_cocktail.name);
    draw_menu_1_tile(i, is_selected, isCocktailAvailable(preset_cocktails[i]));
  }
}

void draw_menu_2() {
  tft.setTextSize(DEFAULT_TEXT_SIZE);
  for (int ingredient_index = 0; ingredient_index < 4; ingredient_index++) {
    bool is_max_amount = current_custom_cocktail.amounts[ingredient_index] >= 200;
    bool can_add = !is_max_amount && isIngredientAvailable(ingredients[ingredient_index], current_custom_cocktail.amounts[ingredient_index] + MENU_2_INGREDIENT_DELTA);
    bool can_subtract = current_custom_cocktail.amounts[ingredient_index] > 0;
    draw_menu_2_tile(ingredient_index, can_add);
  }
  bool is_available = isCocktailAvailable(current_custom_cocktail);
  if (is_available){
    ordered_cocktail = current_custom_cocktail;
  }
}

void draw_menu_3() {
  tft.setTextSize(DEFAULT_TEXT_SIZE);
  tft.setCursor(50, SCREEN_HEIGHT / 2 - 5);
  for (int k = 0; k < INGREDIENT_COUNT; k++) {
    tft.print(ingredients[k].name[0]);
    tft.print(": ");
    tft.print(ingredients[k].amount_left);
    tft.print("\n");
  }
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
    case Error_Screen:
      draw_error_screen();
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

  if (current_menu == Cancellable_Op || current_menu == Error_Screen) {
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

void draw_error_screen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM); // Middle center
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setTextSize(DEFAULT_TEXT_SIZE + 1);
  tft.drawString("Error", CANCEL_MENU_TEXT_CENTER_X, CANCEL_MENU_TEXT_CENTER_Y - 30);

  tft.setTextSize(DEFAULT_TEXT_SIZE);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(current_error_message, CANCEL_MENU_TEXT_CENTER_X, CANCEL_MENU_TEXT_CENTER_Y);

  tft.fillRect(CANCEL_BUTTON_X, CANCEL_BUTTON_Y, CANCEL_BUTTON_SIZE, CANCEL_BUTTON_SIZE, TFT_RED);
  tft.setTextColor(TFT_WHITE, TFT_RED);
  tft.setTextSize(DEFAULT_TEXT_SIZE);
  tft.drawString("X", CANCEL_MENU_TEXT_CENTER_X, CANCEL_BUTTON_Y + CANCEL_BUTTON_SIZE / 2);

  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Back", CANCEL_MENU_TEXT_CENTER_X, CANCEL_BUTTON_Y + CANCEL_BUTTON_SIZE + 10);
}

void handle_touch_side_menu(int x, int y){
  int button = y / SIDE_BUTTON_HEIGHT;
  if (button < 3) {
    current_menu = static_cast<MenuState>(button + 1);
    draw_current_menu();
  } 
  else {
    if (ordered_cocktail.name == UNSELECTED_COCKTAIL_NAME) {
      alert_error("No cocktail selected");
    }
    else {
      order_pending = true;
    }
  }
}

void alert_error(String msg){
  current_error_message = msg;
  current_menu = Error_Screen;
  draw_current_menu();
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
  bool is_curr_available = isCocktailAvailable(preset_cocktails[new_tile]);

  if(!is_curr_available){
    return;
  }

  if (preset_cocktails[new_tile].name != current_preset_cocktail.name) {
    int prev_tile = -1;
    for (int i = 0; i < TABLE_DIMENSION * TABLE_DIMENSION; i++) {
      if (preset_cocktails[i].name == current_preset_cocktail.name) {
        prev_tile = i;
        break;
      }
    }

    bool is_prev_available = isCocktailAvailable(preset_cocktails[prev_tile]);
    if (prev_tile != -1) draw_menu_1_tile(prev_tile, false, is_prev_available);
    draw_menu_1_tile(new_tile, true, is_curr_available);

    //if we got here it's available
    current_preset_cocktail.name = preset_cocktails[new_tile].name;
    memcpy(current_preset_cocktail.amounts, preset_cocktails[new_tile].amounts, sizeof(current_preset_cocktail.amounts));
    ordered_cocktail = current_preset_cocktail;

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
  int ingredient_index = cj * 2 + ci;

  int lx = x % tileW;
  int ly = y % tileH;

  int bx = (tileW - button_total) / 2;
  int by = tileH - button_size - 5;

  bool changed = false;
  if (lx >= bx && lx < bx + button_size && ly >= by && ly < by + button_size) {
    int maximal_ingredient_available = min(MAX_COCKTAIL_DRINK_AMOUNT, int(ingredients[ingredient_index].amount_left));
    current_custom_cocktail.amounts[ingredient_index] = min(current_custom_cocktail.amounts[ingredient_index] + MENU_2_INGREDIENT_DELTA, maximal_ingredient_available);
    changed = true;
  } 

  else if (lx >= bx + button_size + spacing && lx < bx + button_total && ly >= by && ly < by + button_size) {
    current_custom_cocktail.amounts[ingredient_index] = max(0, current_custom_cocktail.amounts[ingredient_index] - MENU_2_INGREDIENT_DELTA);
    changed = true;
  }

  bool is_max_amount = current_custom_cocktail.amounts[ingredient_index] >= MAX_COCKTAIL_DRINK_AMOUNT;
  bool can_add = !is_max_amount && isIngredientAvailable(ingredients[ingredient_index], current_custom_cocktail.amounts[ingredient_index] + MENU_2_INGREDIENT_DELTA);
  if (changed) {
    ordered_cocktail = current_custom_cocktail;
    draw_menu_2_tile(ingredient_index, can_add);
  }

  bool is_available = isCocktailAvailable(current_custom_cocktail);
  if (is_available){
    ordered_cocktail = current_custom_cocktail;
  }
}


void handle_touch_menu_3(int x, int y){
  for (int k = 0; k < INGREDIENT_COUNT; k++) {
    ingredients[k].amount_left=500;
  }
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