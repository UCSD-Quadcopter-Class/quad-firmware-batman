#include "radio.h"

int motorPin = 8; // B5
int throttle = 127;

void setup()
{
  Serial.begin(9600);
  rfBegin(19);
  pinMode(motorPin, OUTPUT);
}

void loop()
{
  if (rfAvailable())
  {
    unsigned char tmp = rfRead();
    Serial.println(int (tmp)); // issue: reads a byte...
    if(tmp >= 0 && tmp <= 255) { // make sure received value is in range
      throttle = tmp;
    } else {
      Serial.print("Not in range: ");
      Serial.println(tmp);
      throttle = 0;
    }
  }

  //Serial.print("Throttle: ");
  //Serial.println(throttle);
  analogWrite(motorPin, throttle);
}

