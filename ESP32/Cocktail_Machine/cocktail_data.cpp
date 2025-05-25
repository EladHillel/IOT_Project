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

bool isCocktailAvailable(Cocktail cocktail) {
  for (int ingredientIndex = 0; ingredientIndex < INGREDIENT_COUNT; ingredientIndex++) {
    float required = cocktail.amounts[ingredientIndex];
    if (!isIngredientAvailable(ingredients[ingredientIndex], required)) {
      return false;
    }
  }
  Serial.println("Cocktail is available.");
  return true;
}

bool isIngredientAvailable(Ingredient ingredient ,float required) {
  const float MINIMUM_INGREDIENT_AMOUNT_THRESHOLD = 5;
  int available = ingredient.amount_left;

    Serial.print("Ingredient ");
    Serial.print(ingredient.name);
    Serial.print(": Required = ");
    Serial.print(required);
    Serial.print(" ml, Available = ");
    Serial.print(available);
    Serial.println(" ml");

    if (required > available - MINIMUM_INGREDIENT_AMOUNT_THRESHOLD) {
      Serial.println(" -> Not enough available.");
      return false;
    }
    return true;
}

bool isCocktailEmpty(Cocktail cocktail){
    for(int ingredient = 0; ingredient < INGREDIENT_COUNT; ingredient++){
      if (cocktail.amounts[ingredient] != 0 ){
        return false;
      }
  }
  return true;
}

void log_cocktail(const Cocktail& cocktail) {
  Serial.print("Cocktail: ");
  Serial.println(cocktail.name);
  for (int i = 0; i < INGREDIENT_COUNT; i++) {
    Serial.print("  Ingredient ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(cocktail.amounts[i]);
    Serial.println(" ml");
  }
}