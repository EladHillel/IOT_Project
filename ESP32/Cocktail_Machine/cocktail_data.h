#ifndef COCKTAIL_DATA_H
#define COCKTAIL_DATA_H

#include <TFT_eSPI.h>

const int INGREDIENT_COUNT = 4;

struct Cocktail {
  String name;
  int amounts[INGREDIENT_COUNT];
};

struct Ingredient {
  const String name;
  uint16_t color;
};

extern const Cocktail cocktails[];
extern const int cocktailCount;
extern const Ingredient ingredients[];

extern Cocktail current_custom_cocktail;
extern Cocktail current_preset_cocktail;
extern Cocktail ordered_cocktail;
extern bool order_pending;
#endif