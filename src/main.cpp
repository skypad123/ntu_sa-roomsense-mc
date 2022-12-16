//#include <Arduino.h>
 #include <DHT.h>
 #include <DHT_U.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"

// #include <s8_uart.h>
#include <SCD30.h>

#include <ArduinoJson.h>

#define RED_LED 13
#define YELLOW_LED 12
#define GREEN_LED 11

#define MOTION_SENSOR 10

#define S8_RX 17
#define S8_TX 16

#define DHT11_PIN A0
#define DHT11_TYPE DHT11

DHT dht(DHT11_PIN,DHT11_TYPE);

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);


// SoftwareSerial S8_serial(S8_RX,S8_TX);
// S8_UART *sensor_S8;
// S8_sensor sensor;

int pir_state = LOW;  
int sensing_state = 0;

unsigned long time;
float result[3] = {0};
float heatIndex = 0;
StaticJsonDocument<64> doc;

void configureSensor(void);

void setup() {

    

    Wire.begin();
    // Configure serial port, we need it for debug

    Serial.begin(115200);


    // others 
    pinMode(RED_LED,OUTPUT);
    pinMode(YELLOW_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);



    // temp/humidity sensor
     dht.begin();

    // light sensor
    if (tsl.begin()) {
        //Serial.println(F("Found a TSL2591 sensor"));
    } else {
        //Serial.println(F("No sensor found ... check your wiring?"));
        while (1);
    }
    configureSensor();

    // motion sensor
    pinMode(MOTION_SENSOR, INPUT);

    scd30.initialize();
    scd30.setAutoSelfCalibration(1);    
    // pinMode(S8_RX,INPUT);
    // pinMode(S8_TX,OUTPUT);

    // co2 sensor
    // // Initialize S8 sensor
    // S8_serial.begin(S8_BAUDRATE);
    // sensor_S8 = new S8_UART(S8_serial);

    // Check if S8 is available
    // sensor_S8->get_firmware_version(sensor.firm_version);
    // int len = strlen(sensor.firm_version);
    // while (len == 0) {
    //     sensor_S8->get_firmware_version(sensor.firm_version);
    //     len = strlen(sensor.firm_version); 
    //     Serial.println("SenseAir S8 CO2 sensor not found!");
    //     delay(1);
    // }

    // // Show basic S8 sensor info
    // Serial.println(">>> SenseAir S8 NDIR CO2 sensor <<<");
    // printf("Firmware version: %s\n", sensor.firm_version);
    // sensor.sensor_id = sensor_S8->get_sensor_ID();
    // Serial.print("Sensor ID: 0x"); printIntToHex(sensor.sensor_id, 4); Serial.println("");

    // Serial.println("Setup done!");
    // Serial.flush();

    time = millis();
}


void loop() {

    sensing_state = digitalRead(MOTION_SENSOR);
    if (sensing_state == HIGH){
        digitalWrite(RED_LED, HIGH);
        if (pir_state == LOW) pir_state = HIGH;
    } 
    else {
        digitalWrite(RED_LED, LOW); // turn LED OFF
        if (pir_state == HIGH) pir_state = LOW;
    }

    if ( millis() - 1000 > time) {

        // Serial.print(F("[ ")); Serial.print(millis());Serial.print(F(" ms ] ")); 
        // if (pir_state == HIGH){
        //     Serial.print("motion   @ "); Serial.println("true");
        //     //Serial.println("Motion detected!");	// print on output change
        // }
        // if (pir_state == LOW){
        //     Serial.print("motion   @ "); Serial.println("false");
        //     //Serial.println("No Motion");	// print on output change
        // } 

        doc["Motion"] = pir_state;
        
        if (scd30.isAvailable()) {
            scd30.getCarbonDioxideConcentration(result);

            // Serial.print(F("[ ")); Serial.print(millis());Serial.print(F(" ms ] "));
            // Serial.print("c02      @ "); Serial.print(result[0]); Serial.println(" ppm");

            // Serial.print(F("[ ")); Serial.print(millis());Serial.print(F(" ms ] ")); 
            // Serial.print("temp     @ "); Serial.print(result[1]); Serial.println(" celsius");
            
            // Serial.print(F("[ ")); Serial.print(millis());Serial.print(F(" ms ] "));
            // Serial.print("humid    @ "); Serial.print(result[2]); Serial.println(" percent"); 

            heatIndex = dht.computeHeatIndex(result[1],result[2]);

            // Serial.print(F("[ ")); Serial.print(millis());Serial.print(F(" ms ] "));
            // Serial.print("heat     @ "); Serial.print(heatIndex); Serial.println(" celsius");

            
            doc["HeatIndex"] = heatIndex ;
            doc["C02"] = result[0];
            doc["Temperature"] = result[1];
            doc["Humidity"] = result[2];
        }

        
        sensors_event_t event;

        tsl.getEvent(&event);
        //Serial.print(F("[ ")); Serial.print(event.timestamp); Serial.print(F(" ms ] "));
        if ((event.light == 0) |
            (event.light > 4294966000.0) | 
            (event.light <-4294966000.0))
        {
            /* If event.light = 0 lux the sensor is probably saturated */
            /* and no reliable data could be generated! */
            /* if event.light is +/- 4294967040 there was a float over/underflow */
            Serial.println(F("Invalid data (adjust gain or timing)"));
        }
        else
        {
            //Serial.print("light    @ ");Serial.print(event.light); Serial.println(F(" lux"));
            doc["Light"] = event.light;
        } 

        time = millis();
        doc["Time"] = time;
        serializeJson(doc, Serial);  
        // Serial.println("");
    }




}

void configureSensor(void)
{
  // You can change the gain on the fly, to adapt to brighter/dimmer light situations
  //tsl.setGain(TSL2591_GAIN_LOW);    // 1x gain (bright light)
  tsl.setGain(TSL2591_GAIN_MED);      // 25x gain
  //tsl.setGain(TSL2591_GAIN_HIGH);   // 428x gain
  
  // Changing the integration time gives you a longer time over which to sense light
  // longer timelines are slower, but are good in very low light situtations!
  //tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  // shortest integration time (bright light)
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
  tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);  // longest integration time (dim light)

  /* Display the gain and integration time for reference sake */  
//   Serial.println(F("------------------------------------"));
//   Serial.print  (F("Gain:         "));
  tsl2591Gain_t gain = tsl.getGain();
  switch(gain)
  {
    case TSL2591_GAIN_LOW:
      //Serial.println(F("1x (Low)"));
      break;
    case TSL2591_GAIN_MED:
      //Serial.println(F("25x (Medium)"));
      break;
    case TSL2591_GAIN_HIGH:
      //Serial.println(F("428x (High)"));
      break;
    case TSL2591_GAIN_MAX:
      //Serial.println(F("9876x (Max)"));
      break;
  }
//   Serial.print  (F("Timing:       "));
//   Serial.print((tsl.getTiming() + 1) * 100, DEC); 
//   Serial.println(F(" ms"));
//   Serial.println(F("------------------------------------"));
//   Serial.println(F(""));
}