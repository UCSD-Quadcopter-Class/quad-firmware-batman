#include <serLCD.h>
#include "radio.h"

const int throttleMin = 112;
const int throttleMax = 840;

int throttle = 0;
int newThrot = 0;

serLCD lcd;

void setup()
{
  Serial.begin(9600);
  //rfBegin(19);
  //lcd.leftToRight();
}

// pins:
// 0: yaw
// 1: throttle
// 2: roll
// 3: pitch

void loop()
{
  throttle = analogRead(1);
  // FIXME range conversion to 0-1500. Calculation is correct, but Ardino not doing it properly
  newThrot = (((throttle - throttleMin) * 1500) / (throttleMax-throttleMin));
  Serial.print("Raw throttle: ");
  Serial.print(throttle);
  Serial.print("\tscaled throttle: ");
  Serial.println(newThrot);
  //rfWrite(throttle); // issue: this only writes a byte (up to 255)
  /*lcd.clear();
  lcd.home();
  lcd.print("ab");*/
}

