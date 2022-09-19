/*

  BLE_Peripheral.ino


  This program is based on the perepherial device mentioned here
https://docs.arduino.cc/tutorials/nano-33-ble-sense/ble-device-to-device
   
++Connecting Nano 33 BLE Devices over Bluetooth++
It uses the Nano Sense as a perepherial device in a bluetooth service called 'magDetectService'.
It flashes a RED LED, if the Server or central device, a ESP32 and its Hall Sensor, detects a magnetic field
using the Hall effect. Else it will flash a constant green LED.

  The circuit:

  - Arduino Nano 33 BLE. 


  This example code is in the public domain.

*/


#include <ArduinoBLE.h>

      

enum {

  MAGNETIC_FALSE = 0,

  MAGNETIC_TRUE = 1,

};


const char* deviceServiceUuid = "19b10000-e8f2-537e-4f6c-d104768a1214";

const char* deviceServiceCharacteristicUuid = "19b10001-e8f2-537e-4f6c-d104768a1214";


int magdetect = 0;


BLEService magDetectService(deviceServiceUuid); 

BLEByteCharacteristic magDetectCharacteristic(deviceServiceCharacteristicUuid, BLERead | BLEWrite);



void setup() {

  Serial.begin(9600);

  while (!Serial);  

  

  pinMode(LEDR, OUTPUT);

  pinMode(LEDG, OUTPUT);

  //pinMode(LEDB, OUTPUT);

  pinMode(LED_BUILTIN, OUTPUT);

  

  digitalWrite(LEDR, HIGH);

  digitalWrite(LEDG, HIGH);

  //digitalWrite(LEDB, HIGH);

  //digitalWrite(LED_BUILTIN, LOW);


  

  if (!BLE.begin()) {

    Serial.println("- Starting Bluetooth® Low Energy module failed!");

    while (1);

  }


  BLE.setLocalName("Arduino Nano 33 BLE (Peripheral)");

  BLE.setAdvertisedService(magDetectService);

  magDetectService.addCharacteristic(magDetectCharacteristic);

  BLE.addService(magDetectService);

  magDetectCharacteristic.writeValue(0);

  BLE.advertise();


  Serial.println("Nano 33 BLE (Peripheral Device)");

  Serial.println(" ");

}


void loop() {

  BLEDevice central = BLE.central();

  Serial.println("- Discovering central device...");

  delay(10000);


  if (central) {

    Serial.println("* Connected to central device!");

    Serial.print("* Device MAC address: ");

    Serial.println(central.address());

    Serial.println(" ");


    while (central.connected()) {

      if (magDetectCharacteristic.written()) {

         magdetect = magDetectCharacteristic.value();

         writeDetection(magdetect);

       }

    }

    

    Serial.println("* Disconnected to central device!");

  }

}


void writeDetection(int magdetect) {

  Serial.println("- Characteristic <magnetic_type> has changed!");

  

   switch (magdetect) {

      case MAGNETIC_TRUE:

        Serial.println("* Actual value: TRUE (red LED on)");

        Serial.println(" ");

        digitalWrite(LEDR, LOW);

        digitalWrite(LEDG, HIGH);

        digitalWrite(LEDB, HIGH);

        digitalWrite(LED_BUILTIN, LOW);

        break;

      default:

        digitalWrite(LEDR, HIGH);

        digitalWrite(LEDG, LOW);

        digitalWrite(LEDB, HIGH);

        digitalWrite(LED_BUILTIN, LOW);

        break;

    }      

}