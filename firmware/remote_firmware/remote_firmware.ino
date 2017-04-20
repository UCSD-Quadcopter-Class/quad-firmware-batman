#include <serLCD.h>

int val = 0;

serLCD lcd;

void setup()
{
  Serial.begin(9600);
  //lcd.setType(3);
  lcd.leftToRight();
  lcd.clear();
}

// 0: yaw
// 1: throttle
// 2: roll
// 3: pitch

void loop()
{
  val = analogRead(0);    // read the input pin
  Serial.println(val); // yaw
  lcd.print('a');
  lcd.print('b');
}

