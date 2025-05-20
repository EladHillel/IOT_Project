#include "cocktail_data.h"
#include <Arduino.h> 
#include "filesystem.h"

const String UNSELECTED_COCKTAIL_NAME = "UNSELECTED";
const String CUSTOM_COCKTAIL_NAME = "Custom Cocktail";

Cocktail preset_cocktails[PRESET_COCKTAIL_COUNT] = {}; 
Ingredient ingredients[INGREDIENT_COUNT] = {};

Cocktail current_custom_cocktail = {CUSTOM_COCKTAIL_NAME, {0, 0, 0, 0}};
Cocktail current_preset_cocktail = {UNSELECTED_COCKTAIL_NAME, {0, 0, 0, 0}};
Cocktail ordered_cocktail = {UNSELECTED_COCKTAIL_NAME, {0, 0, 0, 0}};
bool order_pending = false;

void update_ingredient_amount(int ingredient_index, float amount_poured){
  ingredients[ingredient_index].amount_left = max(0.0f,ingredients[ingredient_index].amount_left - amount_poured);
  save_ingredients(ingredients);
}

bool isCocktailAvailable(Cocktail cocktail){
  for(int ingredientIndex = 0; ingredientIndex< INGREDIENT_COUNT; ingredientIndex++){
    if(cocktail.amounts[ingredientIndex] > ingredients[ingredientIndex].amount_left - 5){ //added 5 as a safety for small liquid amounts
      return false;
    }
    return true;
  }
}


