#include <Wire.h>
//#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>

//#include <stdio.h>
//#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "BLEDevice.h"


// Global definitions.
#define RED_COLOUR_MAX 255
#define RED_COLOUR_MED 100
#define RED_COLOUR_MIN 10

#define TEMP_MIN 0



// Remote station BLE service to connect to.
static BLEUUID serviceUUID("180F");
// The characteristic of the remote service.
static BLEUUID charUUID("2A19");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* device;

// Vector to save up-to-date sensor data. 
std::vector<int> sensorData{0};

// Temperature last 1 hour values.                  
int T_array [] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

int divider = 0; // divider for taking average
int R_average_value = 0;
long previousMillis = 0;  // last time the sensor value was updated

// Notify callback for BLE connection
static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
  // do nothing, as values are read explicitly
}

class WeatherClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

// BLE connect to server, code adapted from BLE client example of
// ESP32 BLE Arduino BLE_client.
bool connectToServer() {
    Serial.print("Forming a connection to remote station");
    Serial.println(device->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new WeatherClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(device);
    Serial.println(" - Connected to remote station");
  
    // Obtain a reference to the weather service in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find weather service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found weather service");


    // Obtain a reference to the characteristic of weather service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found characteristic");

    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }

    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
    return true;
}

class WeatherAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
      BLEDevice::getScan()->stop();
      device = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;
    }
  }
};

// Helper function to convert from string to integer vector.
std::vector<int> convertToIntArray(std::string input)
{
    std::replace(input.begin(), input.end(), ',', ' ');
    std::istringstream stringReader{ input };
    std::vector<int> result;

    int number;
    while (stringReader >> number)
    {
        result.push_back(number);
    }

    return result;
}

//String forecast(x) ...deleted

// The sensorData vector with index = 0 contains Temperature value.
int get_temperature(){
  return sensorData[0];
}


// Initialize base station capabilities.
void setup() {
  Serial.begin(9600);

  BLEDevice::init("");

  // Obtain a scanner and, do active scan and run for 5 seconds. 
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new WeatherAdvertisedDeviceCallbacks());
  pBLEScan->setWindow(449);
  pBLEScan->setInterval(1349);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false); 
}

void loop() {
  // Do the BLE connection and sensor data receive first.
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the remote BLE Server.");
    } else {
      Serial.println("We have failed to connect to the remote server.");
    }
    doConnect = false;
  }
  
  // Once connected to the remote BLE Server, read sensor data.
  if (connected) {    
    // Read the value of the characteristic, the sensor data.
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      sensorData = convertToIntArray(value);
      Serial.print("The sensor value was: ");
      Serial.println(value.c_str());
    }
  } else if (doScan){
    BLEDevice::getScan()->start(0);  // scan after disconnect
  }
  

  // Get individual sensor data.
  int R = get_temperature();
  
  long currentMillis = millis();
  // if 10000ms/1Min have passed, check the sensor level:
  if (currentMillis - previousMillis >= 10000) {
        previousMillis = currentMillis;
  }

  delay(2000);
}
