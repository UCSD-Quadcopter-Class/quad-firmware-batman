#include "radio.h"

int motorPin = 8; // B5
int throttle = 0;

void setup()
{
  Serial.begin(9600);
  rfBegin(19);
  pinMode(motorPin, OUTPUT);
}

void loop()
{
  /*if (rfAvailable())
  {
    Serial.println(rfRead()); // issue: reads a byte...
    int tmp = rfRead();
    if(tmp >= 0 && tmp <= 1023) { // make sure received value is in range
      throttle = tmp;
    } else {
      Serial.print("Not in range: ");
      Serial.println(tmp);
      throttle = 0;
    }*/
  }
  //Serial.print("Throttle: ");
  //Serial.println(throttle);
  analogWrite(motorPin, throttle);
}

