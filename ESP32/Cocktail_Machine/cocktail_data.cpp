#include "cocktail_data.h"

const String UNSELECTED_COCKTAIL_NAME = "UNSELECTED";
const String CUSTOM_COCKTAIL_NAME = "Custom Cocktail";

const Cocktail cocktails[] = {
  { "Mojito", { 150, 50, 0, 0 } },
  { "Martini", { 100, 0, 100, 0 } },
  { "Margarita", { 50, 0, 0, 150 } },
  { "Old Fashioned", { 0, 150, 50, 0 } },
  { "Daiquiri", { 0, 100, 0, 100 } },
  { "Cosmopolitan", { 0, 0, 150, 50 } },
  { "Pina Colada", { 100, 50, 50, 0 } },
  { "Negroni", { 50, 100, 0, 50 } },
  { "Whiskey Sour", { 0, 0, 20, 20 } }
};
const int cocktailCount = sizeof(cocktails) / sizeof(cocktails[0]);

const Ingredient ingredients[] = {
  { "Red", 0xc800 }, { "Green", 0x34c6 }, { "Blue", 0x74fb }, { "Yellow", 0xe746 }
};

Cocktail current_custom_cocktail = {CUSTOM_COCKTAIL_NAME, {0, 0, 0, 0}};
Cocktail current_preset_cocktail = {UNSELECTED_COCKTAIL_NAME, {0, 0, 0, 0}};
Cocktail ordered_cocktail = {UNSELECTED_COCKTAIL_NAME, {0, 0, 0, 0}};
bool order_pending = false;