#include "cocktail_data.h"

const String UNSELECTED_COCKTAIL_NAME = "UNSELECTED";
const String CUSTOM_COCKTAIL_NAME = "Custom Cocktail";

Cocktail preset_cocktails[PRESET_COCKTAIL_COUNT] = {}; 
Ingredient ingredients[INGREDIENT_COUNT] = {};

Cocktail current_custom_cocktail = {CUSTOM_COCKTAIL_NAME, {0, 0, 0, 0}};
Cocktail current_preset_cocktail = {UNSELECTED_COCKTAIL_NAME, {0, 0, 0, 0}};
Cocktail ordered_cocktail = {UNSELECTED_COCKTAIL_NAME, {0, 0, 0, 0}};
bool order_pending = false;