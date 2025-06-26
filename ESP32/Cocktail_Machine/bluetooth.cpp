#include "bluetooth.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>
#include <map>
#include "menu.h"
#include "filesystem.h"

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define SEND_DELAY 3000

enum RequestType { MENU,
                   STATS,
                   INGREDIENTS,
                   UNKNOWN };

RequestType parseRequestType(const std::string& type) {
    if (type == "Menu") return MENU;
    if (type == "Stats") return STATS;
    if (type == "Stock") return INGREDIENTS;
    return UNKNOWN;
}

BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;
bool deviceConnected = false;
bool oldDeviceConnected = false;

static void clearCocktailAmounts(Cocktail& c) {
    for (int i = 0; i < INGREDIENT_COUNT; ++i) {
        c.amounts[i] = 0;
    }
}

void send_menu_via_ble() {
    if (!deviceConnected || !pCharacteristic) return;

    StaticJsonDocument<512> doc;
    JsonArray cocktailArray = doc.to<JsonArray>();

    for (int i = 0; i < PRESET_COCKTAIL_COUNT; ++i) {
        if (preset_cocktails[i].name.length() > 0) {
            JsonObject cocktailObj = cocktailArray.createNestedObject();
            cocktailObj["name"] = preset_cocktails[i].name;
            JsonArray amountsArray = cocktailObj.createNestedArray("amounts");
            for (int j = 0; j < INGREDIENT_COUNT; ++j) {
                amountsArray.add(preset_cocktails[i].amounts[j]);
            }
        }
    }

    String jsonString;
    serializeJson(doc, jsonString);
    pCharacteristic->setValue(jsonString.c_str());
}

static void fillCocktailAmountsFromJson(Cocktail& cocktail, JsonArray amountsJson) {
    int count = min(INGREDIENT_COUNT, int(amountsJson.size()));
    for (int i = 0; i < count; ++i) {
        cocktail.amounts[i] = amountsJson[i].as<int>();
    }
}

void parseIngredientsJson(const String& json) {
    StaticJsonDocument<512> doc;
    DeserializationError err = deserializeJson(doc, json);
    if (err) {
        Serial.println("Failed to parse JSON");
        return;
    }

    for (int i = 0; i < INGREDIENT_COUNT; ++i) {
        JsonObject obj = doc.as<JsonArray>()[i];
        ingredients[i].name = obj["name"].as<String>();
        ingredients[i].amount_left = obj["amount"].as<float>();
    }

    save_ingredients(ingredients);
    draw_current_menu();
}

void send_ingredients_via_ble() {
    if (!deviceConnected || !pCharacteristic) return;

    StaticJsonDocument<512> doc;
    JsonArray ingredientArray = doc.to<JsonArray>();

    for (int i = 0; i < INGREDIENT_COUNT; ++i) {
        JsonObject ing = ingredientArray.createNestedObject();
        ing["name"] = ingredients[i].name;
        ing["amount"] = int(ingredients[i].amount_left);
    }

    String jsonString;
    serializeJson(doc, jsonString);
    pCharacteristic->setValue(jsonString.c_str());
}

static void parseCocktailJson(const String& json) {
    StaticJsonDocument<2048> doc;
    DeserializationError err = deserializeJson(doc, json);
    if (err) {
        Serial.println("Failed to parse JSON");
        return;
    }

    if (!doc.is<JsonArray>()) {
        Serial.println("Expected JSON array");
        return;
    }

    // Backup old cocktails before overwriting
    Cocktail old_presets[PRESET_COCKTAIL_COUNT];
    for (int i = 0; i < PRESET_COCKTAIL_COUNT; ++i) {
        old_presets[i] = preset_cocktails[i];
    }

    int cocktailCount = 0;

    for (JsonObject obj : doc.as<JsonArray>()) {
        if (cocktailCount >= PRESET_COCKTAIL_COUNT) break;

        Cocktail& c = preset_cocktails[cocktailCount];
        c.name = obj["name"].as<String>();
        clearCocktailAmounts(c);

        JsonArray amountsJson = obj["amounts"].as<JsonArray>();
        fillCocktailAmountsFromJson(c, amountsJson);
        cocktailCount++;
    }

    // Reset stats for replaced cocktails
    reset_stats_if_replaced(old_presets, preset_cocktails, stats);

    Serial.println("Parsed cocktails:");
    for (int i = 0; i < cocktailCount; ++i) {
        log_cocktail(preset_cocktails[i]);
    }

    save_cocktails(preset_cocktails, cocktailCount);
    reset_menu_selection();
}

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* s) override {
        deviceConnected = true;
    }
    void onDisconnect(BLEServer* s) override {
        deviceConnected = false;
    }
};

void send_stats_via_ble() {
    if (!deviceConnected || !pCharacteristic) return;

    StaticJsonDocument<512> doc;

    doc["orders_completed"] = stats.orders_completed;
    doc["random_drink_orders"] = stats.random_drink_orders;
    doc["preset_drink_orders"] = stats.preset_drink_orders;
    doc["orders_timed_out"] = stats.orders_timed_out;
    doc["orders_cancelled"] = stats.orders_cancelled;
    doc["custom_drink_orders"] = stats.custom_drink_orders;

    JsonArray orderCountsArray = doc.createNestedArray("preset_cocktail_order_counts");
    for (int i = 0; i < PRESET_COCKTAIL_COUNT; ++i) {
        orderCountsArray.add(stats.preset_cocktail_order_counts[i]);
    }

    String jsonString;
    serializeJson(doc, jsonString);
    pCharacteristic->setValue(jsonString.c_str());
}

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* c) override {
        std::string input = c->getValue();
        if (input.empty()) return;

        size_t first_space = input.find(' ');
        if (first_space == std::string::npos) {
            Serial.println("Malformed input: no prefix/type delimiter");
            return;
        }

        const std::string prefix_request = "REQUEST";
        const std::string prefix_post = "POST";

        std::string prefix = input.substr(0, first_space);

        if (prefix == prefix_request) {
            std::string result = "GOT " + input;
            Serial.println(result.c_str());

            std::string type = input.substr(first_space + 1);

            switch (parseRequestType(type)) {
                case MENU: send_menu_via_ble(); break;
                case STATS: send_stats_via_ble(); break;
                case INGREDIENTS: send_ingredients_via_ble(); break;
                default:
                    char s[512], *p = "0123456789ABCDEF";
                    for (int i = 0; i < 512; i++)
                        s[i] = p[i % 16];
                    c->setValue(s);
                    break;
            }
            return;
        }

        if (prefix == prefix_post) {

            size_t second_space = input.find(' ', first_space + 1);
            if (second_space == std::string::npos) {
                Serial.println("Malformed input: no type/json delimiter");
                return;
            }

            std::string type = input.substr(first_space + 1, second_space - first_space - 1);
            std::string json = input.substr(second_space + 1);

            Serial.println("Received Type: \"" + String(type.c_str()) + "\"");

            Serial.println("Received POST:");
            Serial.println(json.c_str());

            if (type == "Menu")
                parseCocktailJson(String(json.c_str()));
            else if (type == "Stock")
                parseIngredientsJson(String(json.c_str()));
            else
                Serial.println("Unknown POST type");

            return;
        }

        Serial.println("Unknown command format");
    }
};

void ble_setup() {
    Serial.begin(115200);
    BLEDevice::init("ESP32-CocktailBLE");

    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService* pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);
    pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
    pCharacteristic->addDescriptor(new BLE2902());

    pService->start();
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->start();
}

void ble_loop() {
    if (!deviceConnected && oldDeviceConnected) {
        delay(500);
        pServer->startAdvertising();
        oldDeviceConnected = deviceConnected;
    }
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
    }
}