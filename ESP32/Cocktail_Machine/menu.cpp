#include "menu.h"
#include "cocktail_data.h"

TFT_eSPI tft = TFT_eSPI();
SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

MenuState current_menu = Menu_1;
int menu_1_selected_cocktail_tile = -1; 
String current_cancellable_op_text = "";
String current_error_message = "";

int get_menu_1_new_tile(int x,int y){
  return (y / (SCREEN_HEIGHT / 3)) * 3 + (x / (MAIN_WIDTH / 3));
}

bool check_long_press(TS_Point* p) {
  static unsigned long press_start_time = 0;
  static int last_press_tile = -1;
  static bool long_press_reported = false;
  const unsigned long LONG_PRESS_THRESHOLD_MS = 1000; // 2 seconds

  if (p == nullptr) {
    Serial.println("[LongPress] Touch released — resetting state.");
    press_start_time = 0;
    last_press_tile = -1;
    long_press_reported = false;
    return false;
  }

  int tile = get_menu_1_new_tile(p->x, p->y);
  unsigned long now = millis();

  Serial.printf("[LongPress] Current tile: %d | Last tile: %d | Time: %lu\n", tile, last_press_tile, now);

  if (tile == last_press_tile) {
    if (press_start_time == 0) {
      press_start_time = now;
      Serial.printf("[LongPress] Press start time set at %lu for tile %d\n", now, tile);
      return false;
    }

    unsigned long held_time = now - press_start_time;
    Serial.printf("[LongPress] Held for %lu ms\n", held_time);

    if (!long_press_reported && held_time > LONG_PRESS_THRESHOLD_MS) {
      long_press_reported = true;
      Serial.printf("[LongPress] >>> Long press detected on tile %d at %lu\n", tile, now);
      return true;
    }

    return false;
  } else {
    last_press_tile = tile;
    press_start_time = now;
    long_press_reported = false;
    Serial.printf("[LongPress] New tile press detected: %d at %lu\n", tile, now);
    return false;
  }
}



void draw_menu_1_tile(int tile_index, bool selected, bool cocktail_available) {
  Serial.println("draw_menu_1_tile entered");
  const int lineH = 10;
  int i = tile_index % TABLE_DIMENSION;
  int j = tile_index / TABLE_DIMENSION;
  int x = i * (MAIN_WIDTH / TABLE_DIMENSION);
  int y = j * (SCREEN_HEIGHT / TABLE_DIMENSION);

  tft.fillRect(x, y, MAIN_WIDTH / 3, SCREEN_HEIGHT / 3, TFT_BLACK);
  tft.drawRect(x, y, MAIN_WIDTH / 3, SCREEN_HEIGHT / 3, TFT_WHITE);

  Serial.println("printing cocktail on tile");
  tft.setCursor(x + 5, y + 6);
  tft.print(preset_cocktails[tile_index].name);
  for (int k = 0; k < INGREDIENT_COUNT; k++) {
    tft.setCursor(x + 5, y + 10 + (k + 1) * lineH);
    tft.print(ingredients[k].name[0]);
    tft.print(": ");
    tft.print(preset_cocktails[tile_index].amounts[k]);
    tft.print(" ml");
  }

  Serial.println("printing x effect");
  if (!cocktail_available) {
    tft.drawLine(x, y, x + MAIN_WIDTH / 3, y + SCREEN_HEIGHT / 3, TFT_RED);
    tft.drawLine(x + MAIN_WIDTH / 3, y, x, y + SCREEN_HEIGHT / 3, TFT_RED);
    return;
  }

  Serial.println("printing selection effect");
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
    case Cocktail_More:
      draw_side_menu();
      draw_cocktail_more_menu();
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
  Serial.println("\n Handle touch \n");
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
      case Cocktail_More:
        handle_touch_cocktail_extended_menu(x,y);
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
    chosen_cocktail_size = Medium;
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
  Serial.println("Handle touch menu_1");
  int new_tile = get_menu_1_new_tile(x, y);
  bool is_curr_available = isCocktailAvailable(preset_cocktails[new_tile]);

  if(!is_curr_available){
    return;
  }

  //Check if selected tile changed and update view if so
  Serial.printf("new tile %d \n", new_tile);
  Serial.println("Checking selected tile changed");
  if (preset_cocktails[new_tile].name != current_preset_cocktail.name) {
    int prev_tile = -1;
    for (int i = 0; i < TABLE_DIMENSION * TABLE_DIMENSION; i++) {
      Serial.printf("Checking tile %d \n",i);
      if (preset_cocktails[i].name == current_preset_cocktail.name) {
        prev_tile = i;
        Serial.println("Found prev tile:");
        Serial.println(prev_tile);
        break;
      }
    }

    if(prev_tile != -1){
      Serial.println("Prev changed, changing view");
      bool is_prev_available = isCocktailAvailable(preset_cocktails[prev_tile]);
      draw_menu_1_tile(prev_tile, false, is_prev_available);
    }
    draw_menu_1_tile(new_tile, true, is_curr_available);
  }
    //Cocktail is available, change state.
    Serial.println("Changing cocktail state");
    current_preset_cocktail.name = preset_cocktails[new_tile].name;
    memcpy(current_preset_cocktail.amounts, preset_cocktails[new_tile].amounts, sizeof(current_preset_cocktail.amounts));
    ordered_cocktail = current_preset_cocktail;

    Serial.println("New cocktail chosen:");
    Serial.println(current_preset_cocktail.name);
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
  if(current_menu == Menu_1 && check_long_press(p)){
    open_cocktail_more();
  }
}

void return_to_main_menu(){
  current_menu = Menu_1;
  draw_current_menu();
}

void open_cocktail_more(){
  current_menu = Cocktail_More;
  draw_current_menu();
}

void draw_size_button(int x, int y, int w, int h, const char* label, bool highlight) {
    uint16_t btnColor = highlight ? TFT_ORANGE : TFT_DARKGREY;
    tft.fillRoundRect(x, y, w, h, 5, btnColor);
    tft.setTextColor(TFT_WHITE, btnColor);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(label, x + w / 2, y + h / 2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
}

void draw_size_buttons(int y, int btnW, int btnH, int btnGap, int workingWidth) {
    int totalBtnWidth = 3 * btnW + 2 * btnGap;
    int startX = (workingWidth - totalBtnWidth) / 2;

    draw_size_button(startX, y, btnW, btnH, "S", chosen_cocktail_size == Small);
    draw_size_button(startX + btnW + btnGap, y, btnW, btnH, "M", chosen_cocktail_size == Medium);
    draw_size_button(startX + 2 * (btnW + btnGap), y, btnW, btnH, "L", chosen_cocktail_size == Large);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
}

void draw_cocktail_more_menu() {
    tft.fillRect(0, 0, MAIN_WIDTH, SCREEN_HEIGHT, TFT_BLACK);
    draw_size_buttons(20, 60, 40, 20, MAIN_WIDTH);

    // Draw red X at bottom
    int xSize = 40;
    int xX = (MAIN_WIDTH / 2) - (xSize / 2);
    int xY = SCREEN_HEIGHT - xSize - 20;
    tft.fillCircle(xX + xSize / 2, xY + xSize / 2, xSize / 2, TFT_RED);
    tft.drawLine(xX + 10, xY + 10, xX + xSize - 10, xY + xSize - 10, TFT_WHITE);
    tft.drawLine(xX + xSize - 10, xY + 10, xX + 10, xY + xSize - 10, TFT_WHITE);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
}

void handle_touch_cocktail_extended_menu(int x, int y) {
    int btnW = 60, btnH = 40, btnGap = 20;
    int totalBtnWidth = 3 * btnW + 2 * btnGap;
    int workingWidth = MAIN_WIDTH;

    CocktailSize prevSelection = chosen_cocktail_size;
    int btnY = 20;
    int startX = (workingWidth - totalBtnWidth) / 2;

    if (y >= btnY && y <= btnY + btnH) {
        if (x >= startX && x <= startX + btnW) {
            chosen_cocktail_size = Small;
        } else if (x >= startX + btnW + btnGap && x <= startX + 2 * btnW + btnGap) {
            chosen_cocktail_size = Medium;
        } else if (x >= startX + 2 * (btnW + btnGap) && x <= startX + 3 * btnW + 2 * btnGap) {
            chosen_cocktail_size = Large;
        }
    }
    // Redraw only previous and current buttons if changed
    if (prevSelection != chosen_cocktail_size) {
        // Draw previous button unhighlighted
        int prevX = startX + (prevSelection * (btnW + btnGap));
        const char* prevLabel = (prevSelection == Small) ? "S" : (prevSelection == Medium) ? "M" : "L";
        draw_size_button(prevX, btnY, btnW, btnH, prevLabel, false);

        // Draw current button highlighted
        int currX = startX + (chosen_cocktail_size * (btnW + btnGap));
        const char* currLabel = (chosen_cocktail_size == Small) ? "S" : (chosen_cocktail_size == Medium) ? "M" : "L";
        draw_size_button(currX, btnY, btnW, btnH, currLabel, true);
        return;
    }

    int xSize = 40;
    int xX = (workingWidth / 2) - (xSize / 2);
    int xY = SCREEN_HEIGHT - xSize - 20;

    if (x >= xX && x <= xX + xSize &&
        y >= xY && y <= xY + xSize) {
        chosen_cocktail_size = Medium;
        return_to_main_menu();
    }
}

