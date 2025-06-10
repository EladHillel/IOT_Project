#include "cocktail_data.h"
#include <Arduino.h> 
#include "filesystem.h"
#include <algorithm>
#include <utility>

const String UNSELECTED_COCKTAIL_NAME = "UNSELECTED";
const String CUSTOM_COCKTAIL_NAME = "Custom Cocktail";

Cocktail preset_cocktails[PRESET_COCKTAIL_COUNT] = {}; 
Cocktail top_cocktails[TOP_COCKTAIL_COUNT] = {}; 
Ingredient ingredients[INGREDIENT_COUNT] = {};
Stats stats;

Cocktail current_custom_cocktail = {CUSTOM_COCKTAIL_NAME, {0, 0, 0, 0}};
Cocktail current_preset_cocktail = {UNSELECTED_COCKTAIL_NAME, {0, 0, 0, 0}};
Cocktail ordered_cocktail = {UNSELECTED_COCKTAIL_NAME, {0, 0, 0, 0}};
CocktailSize chosen_cocktail_size = Medium;
bool order_pending = false;
Mode mode = Normal;

void update_ingredient_amount(int ingredient_index, float amount_poured){
  ingredients[ingredient_index].amount_left = max(0.0f,ingredients[ingredient_index].amount_left - amount_poured);
  save_ingredients(ingredients);
}

bool isCocktailAvailable(Cocktail cocktail) {
  Serial.println("#######Checking if cocktail "+cocktail.name+" is available.#######");
  for (int ingredientIndex = 0; ingredientIndex < INGREDIENT_COUNT; ingredientIndex++) {
    float required = cocktail.amounts[ingredientIndex];
    if (!isIngredientAvailable(ingredients[ingredientIndex], required)) {
      Serial.println("#######Cocktail is unavailable.#########");
      return false;
    }
  }
  Serial.println("#######Cocktail is available.#########");
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

Cocktail get_random_cocktail(){
  Serial.println("Generating random cocktail");
  int capacity_left = MAX_COCKTAIL_DRINK_AMOUNT;
  Cocktail random_cocktail = {RANDOM_COCKTAIL_NAME, {0, 0, 0, 0}};
   for (int i = 0; i < INGREDIENT_COUNT - 1; i++) {
    int curr_ingredient_amount = random(0, min((int)ingredients[i].amount_left, capacity_left));
    Serial.print(" Ingredient ");
    Serial.print(ingredients[i].name);
    Serial.print(": ");
    Serial.print(curr_ingredient_amount);
    Serial.println(" ml");
    capacity_left -= curr_ingredient_amount;
    random_cocktail.amounts[i] = curr_ingredient_amount;
  }
  int last = INGREDIENT_COUNT - 1;
  Serial.print(" Ingredient ");
  Serial.print(ingredients[last].name);
  Serial.print(": ");
  Serial.print(capacity_left);
  Serial.println(" ml");
  random_cocktail.amounts[last] = min(capacity_left, (int)ingredients[last].amount_left);
  return random_cocktail;
}

void deselect_preset_cocktail(){
  current_preset_cocktail.name = UNSELECTED_COCKTAIL_NAME;
  int zero_amounts[INGREDIENT_COUNT] = {0};
  memcpy(current_preset_cocktail.amounts, zero_amounts, sizeof(zero_amounts));
  ordered_cocktail = current_preset_cocktail;
}

void reset_stats_if_replaced(const Cocktail old_presets[], const Cocktail new_presets[], Stats& stats) {
  for (int i = 0; i < PRESET_COCKTAIL_COUNT; i++) {
    if (old_presets[i].name != new_presets[i].name) {
      stats.preset_cocktail_order_counts[i] = 0;
    }
  }
}

void update_stats_on_drink_order(OrderState state) {
  const String& name = ordered_cocktail.name;

  if (name == UNSELECTED_COCKTAIL_NAME) {
    return;  // Ignore unselected
  }

  switch (state) {
    case Completed:
      stats.orders_completed++;
      break;
    case Cancelled:
      stats.orders_cancelled++;
      break;
    case Timeout:
      stats.orders_timed_out++;
      break;
  }

  if (name == CUSTOM_COCKTAIL_NAME) {
    stats.custom_drink_orders++;
    return;
  }

   if (name == RANDOM_COCKTAIL_NAME) {
    stats.random_drink_orders++;
    return;
  }

  // Assume anything else (except unselected/custom) is preset
  stats.preset_drink_orders++;

  // Increment preset cocktail count by index
  for (int i = 0; i < PRESET_COCKTAIL_COUNT; i++) {
    if (preset_cocktails[i].name == name) {
      stats.preset_cocktail_order_counts[i]++;
      return;
    }
  }
}


void update_top_ordered_cocktails() {
  // Pair each index with its order count
  std::pair<int, int> index_count[PRESET_COCKTAIL_COUNT];
  for (int i = 0; i < PRESET_COCKTAIL_COUNT; ++i) {
    index_count[i] = {i, stats.preset_cocktail_order_counts[i]};
  }

  // Sort by count descending
  std::sort(index_count, index_count + PRESET_COCKTAIL_COUNT,
    [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
      return a.second > b.second;
    });

  Serial.println("Top ordered cocktails:");
  for (int i = 0; i < 3; ++i) {
    int idx = index_count[i].first;
    int count = index_count[i].second;
    top_cocktails[i] = preset_cocktails[idx];

    Serial.printf("  %d. %s - %d orders\n", i + 1, preset_cocktails[idx].name.c_str(), count);
  }
}
