#include <serLCD.h>
#include "radio.h"

const long thrMin = 112;
const long thrMax = 840;
const long yawMin = 116;
const long yawMax = 852;
const long pitchMin = 78;
const long pitchMax = 811;
const long rollMin = 115;
const long rollMax = 838;

long thr = 0;
long yaw = 0;
long pitch = 0;
long roll = 0;
long rfThrot = 0;
long sThrot = 0;

serLCD lcd;

void setup()
{
  Serial.begin(9600);
  rfBegin(19);
  lcd.leftToRight();
}

long convertRange(long value, long oldMin, long oldMax, long newMin, long newMax)
{
  return ((value - oldMin) * (newMax-newMin)) / (oldMax-oldMin) + newMin;
}

void serialPrint()
{
  Serial.print("Raw:\t");
  Serial.print(thr);
  Serial.print(" ");
  Serial.print(yaw);
  Serial.print(" ");
  Serial.print(pitch);
  Serial.print(" ");
  Serial.print(roll);
  Serial.print("\tScaled:\t");
  Serial.print(convertRange(thr, thrMin, thrMax, 0, 1500));
  Serial.print(" ");
  Serial.print(convertRange(yaw, yawMin, yawMax, 0, 1500));
  Serial.print(" ");
  Serial.print(convertRange(pitch, pitchMin, pitchMax, 0, 1500));
  Serial.print(" ");
  Serial.print(convertRange(roll, rollMin, rollMax, 0, 1500));
  Serial.print("\n");
}

void lcdPrint()
{
  lcd.clear();
  lcd.selectLine(0);
  lcd.print(convertRange(thr, thrMin, thrMax, 0, 1500));
  lcd.print(" ");
  lcd.print(convertRange(yaw, yawMin, yawMax, 0, 1500));
  lcd.selectLine(1);
  lcd.print(convertRange(pitch, pitchMin, pitchMax, 0, 1500));
  lcd.print(" ");
  lcd.print(convertRange(roll, rollMin, rollMax, 0, 1500));
}

void loop()
{
  thr = analogRead(1);
  yaw = analogRead(0);
  pitch = analogRead(3);
  roll = analogRead(2);
  serialPrint();
  //lcdPrint();
  rfThrot = convertRange(thr, thrMin, thrMax, 0, 255);
  rfYaw = convertRange(yaw, yawMin, yawMax, 0, 255);
  rfPitch = convertRange(pitch, pitchMin, pitchMax, 0, 255);
  rfRoll = convertRange(roll, rollMin, rollMax, 0, 255);
  rfWrite(lowByte(rfThrot)); // TODO guard against out of range (0-255)
  rfWrite(lowByte(rfYaw)); // TODO guard against out of range (0-255)
  rfWrite(lowByte(rfPitch)); // TODO guard against out of range (0-255)
  rfWrite(lowByte(rfRoll)); // TODO guard against out of range (0-255)

}

