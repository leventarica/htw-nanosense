#include <Arduino_HTS221.h>//wahrsch. der Temp-Sensor
#include <ArduinoBLE.h>

//service UUID is 180F
BLEService weatherService("180F");
BLEStringCharacteristic weatherTemp("2A19", BLERead | BLENotify, 30);
String oldTemp = "0";  // last sensor readings
long previousMillis = 0;  // last time the sensor level was read, in ms
float temp = 6.5;

void updateTemp(float R) {
  String sensorLevel = String(R);
  Serial.print("Temperature is now: "); // print sensor readings
  Serial.println(sensorLevel);
  weatherTemp.writeValue(sensorLevel);  // and update the weather level characteristic
}

// Get the temperature value. 
float get_temperature() {
  float temp = HTS.readTemperature(); 
  return temp;
}

void setup() {
  Serial.begin(9600);

  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");
    while (1);
  }
  //initilizing Temp Sensor
  if (!HTS.begin()) {
  Serial.println("Failed to initialize humidity temperature sensor!");
  while (1);
  }
  //Setting up LED to light when connected to a central device. 
  //Source:  Agus Kurniawan in Beginning Arduino Nano 33 IoT (2021) 
  pinMode(LED_BUILTIN, OUTPUT);

  BLE.setLocalName("WeatherMonitor");
  BLE.setAdvertisedService(weatherService); // add the service UUID
  weatherService.addCharacteristic(weatherTemp); // add the temp characteristic
  BLE.addService(weatherService); // Add the battery service
  weatherTemp.writeValue(oldTemp); // set initial value for this characteristic
  
  //start advertising the Service 
  BLE.advertise();
  Serial.println("Bluetooth® Perepherial Arduino Device active, waiting for connections...");
}

void loop() {
  // one sensor reading R
  float R;
  // wait for a BLE central device
  BLEDevice central = BLE.central();

  if (central) {
    // connected to base station
    Serial.print("Found and Connected to central: ");

    // print the base station BT address:
    Serial.println(central.address());

    //Turn on onboard LED's
    digitalWrite(LEDR, LOW);
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDB, HIGH);
    digitalWrite(LED_BUILTIN, HIGH);

    // check the sensor every 500ms while the central is connected:
    while (central.connected()) {
      long currentMillis = millis();
      
      if (currentMillis - previousMillis >= 500) {
        previousMillis = currentMillis;
	      Serial.print("Trying to get Temp Value... ");
        R = (float)get_temperature();
        Serial.print("Temp was found at ");
        Serial.print(R);
        Serial.println ("°C");
        updateTemp(R);
        Serial.println("");
      }
    }
    digitalWrite(LEDR, HIGH);
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDB, HIGH);
    digitalWrite(LED_BUILTIN, LOW);
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
  else {
    Serial.println("Disconnected from central");
    delay(5000);
  }
}