#ifndef COCKTAIL_DATA_H
#define COCKTAIL_DATA_H

#include <TFT_eSPI.h>
#include <map>

const int INGREDIENT_COUNT = 4;
const int PRESET_COCKTAIL_COUNT = 9;

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

void update_ingredient_amount(int ingredient_index, float amount_poured);
#endif