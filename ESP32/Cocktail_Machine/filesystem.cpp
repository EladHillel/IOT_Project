#include "filesystem.h"
#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

bool fs_init() {
  // Initialize the file system
  if (!LittleFS.begin(true)) {
    return false;
  }

  // Check if the necessary files exist, if not, create them
  if (!LittleFS.exists("/cocktails.json")) {
    fs::File file = LittleFS.open("/cocktails.json", "w");
    if (!file) return false;
    StaticJsonDocument<2048> document;
    serializeJson(document, file); 
    file.close();
  }

  if (!LittleFS.exists("/ingredients.json")) {
    fs::File file = LittleFS.open("/ingredients.json", "w");
    if (!file) return false;
    StaticJsonDocument<1024> document;
    serializeJson(document, file); 
    file.close();
  }

  if (!LittleFS.exists("/stats.json")) {
    fs::File file = LittleFS.open("/stats.json", "w");
    if (!file) return false;
    StaticJsonDocument<2048> document;
    serializeJson(document, file); 
    file.close();
  }

  return true;
}

bool save_cocktails(const Cocktail cocktails[], size_t count) {
  if (count > PRESET_COCKTAIL_COUNT) return false;
  fs::File file = LittleFS.open("/cocktails.json", "w");
  if (!file) return false;
  StaticJsonDocument<2048> document;
  JsonArray cocktailArray = document.to<JsonArray>();
  for (size_t i = 0; i < count; ++i) {
    JsonObject cocktailObject = cocktailArray.createNestedObject();
    cocktailObject["name"] = cocktails[i].name;
    JsonArray amountsArray = cocktailObject.createNestedArray("amounts");
    for (int j = 0; j < 4; ++j) amountsArray.add(cocktails[i].amounts[j]);
  }
  serializeJson(document, file);
  file.close();
  return true;
}

bool load_cocktails(Cocktail cocktails[], size_t& count) {
  fs::File file = LittleFS.open("/cocktails.json", "r");
  if (!file) return false;
  StaticJsonDocument<2048> document;
  DeserializationError err = deserializeJson(document, file);
  if (err) return false;
  JsonArray cocktailArray = document.as<JsonArray>();
  count = 0;
  for (JsonObject cocktailObject : cocktailArray) {
    if (count >= PRESET_COCKTAIL_COUNT) break;
    Cocktail& cocktail = cocktails[count++];
    cocktail.name = cocktailObject["name"].as<String>();
    JsonArray amountsArray = cocktailObject["amounts"];
    for (int j = 0; j < 4; ++j) cocktail.amounts[j] = amountsArray[j];
  }
  file.close();
  return true;
}

bool save_ingredients(const Ingredient ingredients[INGREDIENT_COUNT]) {
  fs::File file = LittleFS.open("/ingredients.json", "w");
  if (!file) return false;
  StaticJsonDocument<1024> document;
  JsonArray ingredientArray = document.to<JsonArray>();
  for (int i = 0; i < 4; ++i) {
    JsonObject ingredientObject = ingredientArray.createNestedObject();
    ingredientObject["name"] = ingredients[i].name;
    ingredientObject["color"] = ingredients[i].color;
  }
  serializeJson(document, file);
  file.close();
  return true;
}

bool load_ingredients(Ingredient ingredients[INGREDIENT_COUNT]) {
  fs::File file = LittleFS.open("/ingredients.json", "r");
  if (!file) return false;
  StaticJsonDocument<1024> document;
  DeserializationError err = deserializeJson(document, file);
  if (err) return false;
  JsonArray ingredientArray = document.as<JsonArray>();
  int i = 0;
  for (JsonObject ingredientObject : ingredientArray) {
    if (i >= 4) break;
    ingredients[i].name = ingredientObject["name"].as<String>();
    ingredients[i].color = ingredientObject["color"];
    ++i;
  }
  file.close();
  return true;
}

bool save_stats(const Stats& stats) {
  fs::File file = LittleFS.open("/stats.json", "w");
  if (!file) return false;
  StaticJsonDocument<2048> document;
  JsonObject rootObject = document.to<JsonObject>();
  rootObject["total"] = stats.totalDispenses;
  JsonObject countsObject = rootObject.createNestedObject("counts");
  for (const auto& entry : stats.drinkCounts) {
    countsObject[entry.first] = entry.second;
  }
  serializeJson(document, file);
  file.close();
  return true;
}

bool load_stats(Stats& stats) {
  fs::File file = LittleFS.open("/stats.json", "r");
  if (!file) return false;
  StaticJsonDocument<2048> document;
  DeserializationError err = deserializeJson(document, file);
  if (err) return false;
  stats.totalDispenses = document["total"] | 0;
  stats.drinkCounts.clear();
  JsonObject countsObject = document["counts"];
  for (JsonPair entry : countsObject) {
    stats.drinkCounts[entry.key().c_str()] = entry.value();
  }
  file.close();
  return true;
}

void setup_data(){ 
  while(!fs_init()) {
     Serial.println("Filesystem ran into issue.");
  }

  Serial.println("Filesystem initialized");
  size_t count = 0;
  while(!load_cocktails(preset_cocktails, count)) {
     Serial.println("Loading preset cocktails ran into issue.");
  }
  Serial.println("Loaded " + String(count) + " preset cocktails");
  
  while(!load_ingredients(ingredients)) {
     Serial.println("Loading ingredients ran into issue.");
  }
  Serial.println("Loaded preset ingredients");
}
