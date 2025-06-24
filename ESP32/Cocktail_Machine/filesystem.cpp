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
    for (int i = 0; i < INGREDIENT_COUNT; ++i) {
        JsonObject ingredientObject = ingredientArray.createNestedObject();
        ingredientObject["name"] = ingredients[i].name;
        ingredientObject["color"] = ingredients[i].color;
        ingredientObject["amount_left"] = ingredients[i].amount_left;
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
        if (i >= INGREDIENT_COUNT) break;
        ingredients[i].name = ingredientObject["name"].as<String>();
        ingredients[i].color = ingredientObject["color"];
        ingredients[i].amount_left = ingredientObject["amount_left"] | 0.0f;
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

    // Save individual counters
    rootObject["orders_completed"] = stats.orders_completed;
    rootObject["random_drink_orders"] = stats.random_drink_orders;
    rootObject["preset_drink_orders"] = stats.preset_drink_orders;
    rootObject["orders_timed_out"] = stats.orders_timed_out;
    rootObject["orders_cancelled"] = stats.orders_cancelled;
    rootObject["custom_drink_orders"] = stats.custom_drink_orders;

    // Save preset cocktail order counts as an array
    JsonArray cocktailCounts = rootObject.createNestedArray("preset_cocktail_order_counts");
    for (int i = 0; i < PRESET_COCKTAIL_COUNT; i++) {
        cocktailCounts.add(stats.preset_cocktail_order_counts[i]);
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
    if (err) {
        file.close();
        return false;
    }

    // Load individual counters with default values of 0
    stats.orders_completed = document["orders_completed"] | 0;
    stats.random_drink_orders = document["random_drink_orders"] | 0;
    stats.preset_drink_orders = document["preset_drink_orders"] | 0;
    stats.orders_timed_out = document["orders_timed_out"] | 0;
    stats.orders_cancelled = document["orders_cancelled"] | 0;
    stats.custom_drink_orders = document["custom_drink_orders"] | 0;

    // Load preset cocktail order counts
    JsonArray cocktailCounts = document["preset_cocktail_order_counts"];
    for (int i = 0; i < PRESET_COCKTAIL_COUNT; i++) {
        if (i < cocktailCounts.size()) {
            stats.preset_cocktail_order_counts[i] = cocktailCounts[i] | 0;
        } else {
            stats.preset_cocktail_order_counts[i] = 0;
        }
    }

    file.close();
    return true;
}

void setup_data() {
    while (!fs_init()) {
        Serial.println("Filesystem ran into issue.");
    }

    Serial.println("Filesystem initialized");
    size_t count = 0;
    while (!load_cocktails(preset_cocktails, count)) {
        Serial.println("Loading preset cocktails ran into issue.");
    }
    Serial.println("Loaded " + String(count) + " preset cocktails");

    while (!load_ingredients(ingredients)) {
        Serial.println("Loading ingredients ran into issue.");
    }
    Serial.println("Loaded preset ingredients");
}
