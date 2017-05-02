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

serLCD lcd;

typedef struct {
  int header;
  int thr;
  int yaw;
  int pitch;
  int roll;
} Control;

Control controls;

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
  Serial.print("\t\tScaled:\t");
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
  //lcd.clear();
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
  lcdPrint();

  controls.header = 0xB3EF;
  controls.thr = convertRange(thr, thrMin, thrMax, 0, 1500);
  controls.yaw = convertRange(yaw, yawMin, yawMax, 0, 1500);
  controls.pitch = convertRange(pitch, pitchMin, pitchMax, 0, 1500);
  controls.roll = convertRange(roll, rollMin, rollMax, 0, 1500);

  rfWrite((uint8_t*)&controls, sizeof(Control));
}


