//#include <Arduino.h>
#include <DHT.h>
#include <DHT_U.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"

#define RED_LED 13
#define YELLOW_LED 12
#define GREEN_LED 11

#define MOTION_SENSOR 4

#define DHT11_PIN A0
#define DHT11_TYPE DHT11

DHT dht(DHT11_PIN,DHT11_TYPE);

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);

int pir_state = LOW;  
int sensing_state = 0;

unsigned long time;

void configureSensor(void);

void setup() {

    // Configure serial port, we need it for debug
    Serial.begin(9600);

    pinMode(RED_LED,OUTPUT);
    pinMode(YELLOW_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);

    pinMode(MOTION_SENSOR, INPUT);

    dht.begin();

    if (tsl.begin()) {
        Serial.println(F("Found a TSL2591 sensor"));
    } else {
        Serial.println(F("No sensor found ... check your wiring?"));
        while (1);
    }
    
    /* Configure the sensor */
    configureSensor();

    time = millis();
}


void loop() {

    sensing_state = digitalRead(MOTION_SENSOR);
    if (sensing_state == HIGH){
        digitalWrite(YELLOW_LED, HIGH);
        if (pir_state == LOW) pir_state = HIGH;
    } 
    else {
        digitalWrite(YELLOW_LED, LOW); // turn LED OFF
        if (pir_state == HIGH) pir_state = LOW;
    }

    if ( millis() - 100 > time) {

        Serial.print(F("[ ")); Serial.print(millis());Serial.print(F(" ms ] ")); 
        if (pir_state == HIGH){
            Serial.print("motion   @ "); Serial.println("true");
            //Serial.println("Motion detected!");	// print on output change
        }
        if (pir_state == LOW){
            Serial.print("motion   @ "); Serial.println("false");
            //Serial.println("No Motion");	// print on output change
        } 


        

        
        Serial.print(F("[ ")); Serial.print(millis());Serial.print(F(" ms ] ")); 
        Serial.print("temp     @ "); Serial.print(dht.readTemperature()); Serial.println(" celsius");
        
        Serial.print(F("[ ")); Serial.print(millis());Serial.print(F(" ms ] "));
        Serial.print("humid    @ "); Serial.print(dht.readHumidity()); Serial.println(" percent");

        Serial.print(F("[ ")); Serial.print(millis());Serial.print(F(" ms ] "));
        Serial.print("heat     @ "); Serial.print(dht.computeHeatIndex(dht.readTemperature(),dht.readHumidity())); Serial.println(" celsius");

        
        sensors_event_t event;

        tsl.getEvent(&event);
        Serial.print(F("[ ")); Serial.print(event.timestamp); Serial.print(F(" ms ] "));
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
            Serial.print("light    @ ");Serial.print(event.light); Serial.println(F(" lux"));
        } 

        //dht.humidity().getEvent(&event);
        Serial.print(F("[ ")); Serial.print(millis());Serial.print(F(" ms ] "));
        Serial.print("c02      @ "); Serial.print("100.00"); Serial.println(" ppm");
        

        time = millis();
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
  Serial.println(F("------------------------------------"));
  Serial.print  (F("Gain:         "));
  tsl2591Gain_t gain = tsl.getGain();
  switch(gain)
  {
    case TSL2591_GAIN_LOW:
      Serial.println(F("1x (Low)"));
      break;
    case TSL2591_GAIN_MED:
      Serial.println(F("25x (Medium)"));
      break;
    case TSL2591_GAIN_HIGH:
      Serial.println(F("428x (High)"));
      break;
    case TSL2591_GAIN_MAX:
      Serial.println(F("9876x (Max)"));
      break;
  }
  Serial.print  (F("Timing:       "));
  Serial.print((tsl.getTiming() + 1) * 100, DEC); 
  Serial.println(F(" ms"));
  Serial.println(F("------------------------------------"));
  Serial.println(F(""));
}