#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool readyToSend = false;  // Variable to determine if the ESP32 should send data
uint32_t value = 1;

// UUIDs for the service and characteristic
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// Connection management
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        Serial.println("A device has connected.");
    };

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        Serial.println("A device has disconnected.");
    }
};

// Handling data sent by the application
class MyCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string value_from_app = pCharacteristic->getValue();
        
        if (value_from_app.length() > 0) {
            String receivedData = String(value_from_app.c_str());
            Serial.println("Data received from the application: " + receivedData);
            
            // If the application sends "0", enable data transmission
            if (receivedData == "0") {
                readyToSend = true;
            }
        }
    }
};

void setup() {
    Serial.begin(115200);
    Serial.println("Starting BLE server...");

    // Initialize the BLE device
    BLEDevice::init("ESP32");

    // Create the BLE server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create the BLE service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create the BLE characteristic with read, write, and notify properties
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY |
        BLECharacteristic::PROPERTY_INDICATE
    );

    pCharacteristic->setCallbacks(new MyCallbacks());

    // Start the BLE service
    pService->start();

    // Start BLE advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0);
    BLEDevice::startAdvertising();

    Serial.println("BLE server is ready and waiting for connections...");
}

void loop() {
    // Check if data needs to be sent
    if (deviceConnected && readyToSend) {
        pCharacteristic->setValue((uint8_t*)&value, 4);
        pCharacteristic->notify();
        Serial.println("Notification sent: " + String(value));

        value++;  // Increment the value for the next time
        readyToSend = false;  // Switch back to waiting mode
    }

    // Handle disconnection
    if (!deviceConnected && oldDeviceConnected) {
        delay(500);
        pServer->startAdvertising();
        Serial.println("Restarting BLE advertising...");
        oldDeviceConnected = deviceConnected;
    }

    // Handle reconnection
    if (deviceConnected && !oldDeviceConnected) {
        Serial.println("New connection established.");
        oldDeviceConnected = deviceConnected;
    }

    delay(100);  // Small pause to avoid excessive CPU usage
}
