/*
* Ultrasonic Sensor as DS18B20 thermometer
*/

#include <EEPROM.h>
#include "OneWireHub.h"
#include "DS18B20.h"  

#define ONEWIRE_PIN   2
#define DEVICE_NUMBER 0x01
#define EP_TRIGGERS 0x10

// defines pins numbers
const int trigPin = 9;
const int echoPin = 10;

auto hub = OneWireHub(ONEWIRE_PIN);

auto ds18b20 = DS18B20(DS18B20::family_code, 0xDE, 0xAF, 0x00, DEVICE_NUMBER, 0x00, 0x00);


void setup() {
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(LED_BUILTIN, OUTPUT);
  
  hub.attach(ds18b20);
  ds18b20.setLowTrigger(EEPROM.read(EP_TRIGGERS));
  ds18b20.setHighTrigger(EEPROM.read(EP_TRIGGERS + 1));
 
  for (uint8_t  i = 0; i < 32; ++i)
    continousMeasure(singleMeasure((uint8_t)ds18b20.getHighTrigger())); 

}

uint16_t singleMeasure(uint8_t halfSoundSpeed)
{
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  hub.poll();
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  hub.poll();
  digitalWrite(trigPin, LOW);

  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  uint16_t duration = pulseIn(echoPin, HIGH);
  
  // Calculating the distance
  return ((uint32_t)duration * (uint32_t)halfSoundSpeed) / 1000UL;
}

uint16_t continousMeasure(uint16_t value)
{
  static uint16_t values[16] = {0};
  static uint32_t sum = 0;
  static uint8_t pos = 0;

  sum -= values[pos];
  values[pos] = value;
  sum += values[pos];
 
  pos++;
  if (pos > 15) pos = 0;

  return sum;
}


void loop() {
  hub.poll();
  
  
  if (interval()) {
    EEPROM.update(EP_TRIGGERS, ds18b20.getLowTrigger());
    EEPROM.update(EP_TRIGGERS + 1, ds18b20.getHighTrigger());
    
    int16_t level = (uint8_t)ds18b20.getLowTrigger();
    level <<= 4; // move to fixed point

    digitalWrite(LED_BUILTIN, HIGH);
    uint16_t distance = singleMeasure((uint8_t)ds18b20.getHighTrigger());
    uint16_t distance_avg = continousMeasure(distance);
    ds18b20.setTemperatureRaw(level - ((int16_t)distance_avg/10));  
    digitalWrite(LED_BUILTIN, LOW);  
  }
}


bool interval(void)
{
    constexpr  uint32_t interval    = 1000;
    static uint32_t nextMillis  = millis();

    if (millis() > nextMillis)
    {
        nextMillis += interval;
        return 1;
    }
    return 0;
}
