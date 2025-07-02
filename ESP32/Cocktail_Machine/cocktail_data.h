#ifndef COCKTAIL_DATA_H
#define COCKTAIL_DATA_H

#include <TFT_eSPI.h>
#include <map>

enum Mode {
  Normal,
  Clean,
  Fast
};

enum CocktailSize {
  Small = 0,
  Medium = 1,
  Large = 2
};

enum OrderState {
  Completed,
  Cancelled,
  Timeout
};

const int INGREDIENT_COUNT = 4;
const int PRESET_COCKTAIL_COUNT = 9;
const int MAX_COCKTAIL_DRINK_AMOUNT = 100;
const int POPULAR_DRINK_COUNT = 3;
extern const String UNSELECTED_COCKTAIL_NAME;
const String RANDOM_COCKTAIL_NAME = "Random Cocktail";
const int TOP_COCKTAIL_COUNT = 3;
const float MINIMUM_INGREDIENT_AMOUNT_THRESHOLD = 10;

struct Cocktail {
  String name;
  int amounts[INGREDIENT_COUNT];
};

struct Ingredient {
  String name;
  uint16_t color;
  float amount_left;
};

struct Stats {
  int orders_completed = 0;
  int random_drink_orders = 0;
  int preset_drink_orders = 0;
  int orders_timed_out = 0;
  int orders_cancelled = 0;
  int custom_drink_orders = 0;
  int preset_cocktail_order_counts[PRESET_COCKTAIL_COUNT] = {0};
};

extern Stats stats;

extern Cocktail preset_cocktails[];
extern Ingredient ingredients[];
extern Cocktail top_cocktails[];

extern Cocktail current_custom_cocktail;
extern Cocktail current_preset_cocktail;
extern Cocktail ordered_cocktail;
extern bool order_pending;
extern CocktailSize chosen_cocktail_size; 

void update_ingredient_amount(int ingredient_index, float amount_poured);
bool isCocktailAvailable(Cocktail cocktail);
bool isIngredientAvailable(Ingredient ingredient ,float required);
bool isCocktailEmpty(Cocktail cocktail);
void log_cocktail(const Cocktail& cocktail);
Cocktail get_random_cocktail();
void deselect_preset_cocktail();
void reset_stats_if_replaced(const Cocktail old_presets[], const Cocktail new_presets[], Stats& stats);
void update_stats_on_drink_order(Cocktail cocktail, OrderState state);
void update_top_ordered_cocktails();
#endif
