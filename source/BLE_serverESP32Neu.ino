/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara

    Code was modified on 220918 by jfk
    to meet criteria for using HAll-Sensor as MAgDetection and 
    sending data to peripherie Arduino Nano Sense which will change it's 
    LED to red if magDetect is True.
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h> //added by jfk. check if really necessary
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/


bool deviceConnected = false;

#define SERVICE_UUID        "19b10000-e8f2-537e-4f6c-d104768a1214"
#define CHARACTERISTIC_UUID "19b10000-e8f2-537e-4f6c-d104768a1214"

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;

// Hall Sensor Readings Characteristic and Descriptor
BLECharacteristic magneticFieldCharacteristics("19b10000-e8f2-537e-4f6c-d104768a1214", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor magneticFieldDescriptor(BLEUUID((uint16_t)0x2903));

//Setup callbacks onConnect and onDisconnect
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  // Create the BLE Device
  BLEDevice::init("ESP32asBLE-Server");
  
  // Create the BLE Server: Datatype *varNAme    = Object or Method
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *magDetectService = pServer->createService(SERVICE_UUID);

  // Create BLE Characteristics
  /*
  BLECharacteristic *pCharacteristic = magDetectService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setValue("Hello World says Neil");
  */
  
  magDetectService->addCharacteristic(&magneticFieldCharacteristics);
  magneticFieldDescriptor.setValue("Hall Mag Detection");
  magneticFieldCharacteristics.addDescriptor(new BLE2902());
  
  
  //start the service
  magDetectService->start();

  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Waiting a client connection to notify...");
}
/*
void loop() {
  // put your main code here, to run repeatedly:
  delay(2000);
}
*/
void loop() {
  if (deviceConnected) {
    if ((millis() - lastTime) > timerDelay) {
      // Read Hall
      int val = hallRead();
      Serial.print("Magnetic Field detected: ");
      Serial.println(val);
      lastTime = millis();
    }
  }
  else {
    Serial.println("No Device in paired.");
    delay(10000);    
  }
}