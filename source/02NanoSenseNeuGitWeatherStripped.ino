#include <Arduino_HTS221.h>//wahrsch. der Temp-Sensor
#include <ArduinoBLE.h>

//service UUID is 180F
BLEService weatherService("180F");
BLEStringCharacteristic weatherChar("2A19", BLERead | BLENotify, 30);
String oldSensorLevel = "0";  // last sensor readings
long previousMillis = 0;  // last time the sensor level was read, in ms


void setup() {
  Serial.begin(9600);

  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");
    while (1);
  }
  
  BLE.setLocalName("WeatherMonitor");
  BLE.setAdvertisedService(weatherService); // add the service UUID
  weatherService.addCharacteristic(weatherChar); // add the battery level characteristic
  BLE.addService(weatherService); // Add the battery service
  weatherChar.writeValue(oldSensorLevel); // set initial value for this characteristic

  BLE.advertise();
  Serial.println("Bluetooth® Perepherial Arduino Device active, waiting for connections...");
}

void loop() {
  // one sensor reading R
  int R;
  BLEDevice central = BLE.central();
  if (central) {
    // connected to base station
    Serial.print("Found and Connected to central: ");

    // print the base station BT address:
    Serial.println(central.address());
  
    // check the sensor every 500ms while the central is connected:
    while (central.connected()) {
      long currentMillis = millis();

      if (currentMillis - previousMillis >= 500) {
        previousMillis = currentMillis;

        R = (int)get_temperature();
        
        updateSensorLevel(R);
        Serial.println("");
      }
    }
  }
}

void updateSensorLevel(int R) {
  String sensorLevel = String(R);
  Serial.print("Sensor Level % is now: "); // print sensor readings
  Serial.println(sensorLevel);
  weatherChar.writeValue(sensorLevel);  // and update the weather level characteristic
}

// Get the temperature value. 
float get_temperature() {
  float temperature = HTS.readTemperature();

  Serial.print("Temperature = ");
  Serial.print(temperature);
  Serial.println(" °C");
  return temperature;
}