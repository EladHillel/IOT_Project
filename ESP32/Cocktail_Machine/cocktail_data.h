#ifndef COCKTAIL_DATA_H
#define COCKTAIL_DATA_H

#include <TFT_eSPI.h>
#include <map>


enum CocktailSize {
  Small = 0,
  Medium = 1,
  Large = 2
};

const int INGREDIENT_COUNT = 4;
const int PRESET_COCKTAIL_COUNT = 9;
const int MAX_COCKTAIL_DRINK_AMOUNT = 200;
extern const String UNSELECTED_COCKTAIL_NAME;

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
  std::map<String, uint32_t> drinkCounts;
  uint32_t totalDispenses = 0;
};

extern Cocktail preset_cocktails[];
extern Ingredient ingredients[];

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
#endif