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
byte send[12];
int mask = 0x000000FF;

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
//  Serial.print("Raw:\t");
//  Serial.print(thr);
//  Serial.print(" ");
//  Serial.print(yaw);
//  Serial.print(" ");
//  Serial.print(pitch);
//  Serial.print(" ");
//  Serial.print(roll);
//  Serial.print("\tScaled:\t");
//  Serial.print(convertRange(thr, thrMin, thrMax, 0, 1500));
//  Serial.print(" ");
//  Serial.print(convertRange(yaw, yawMin, yawMax, 0, 1500));
//  Serial.print(" ");
//  Serial.print(convertRange(pitch, pitchMin, pitchMax, 0, 1500));
//  Serial.print(" ");
//  Serial.print(convertRange(roll, rollMin, rollMax, 0, 1500));
//  Serial.print("\n");

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

/*void sendRF()
{
  int thr_i = convertRange(thr, thrMin, thrMax, 0, 1500); // TODO guard against out of range (0-255)
  int yaw_i = convertRange(yaw, yawMin, yawMax, 0, 1500); // TODO guard against out of range (0-255)
  int pit_i = convertRange(pitch, pitchMin, pitchMax, 0, 1500); // TODO guard against out of range (0-255)
  int rol_i = convertRange(roll, rollMin, rollMax, 0, 1500); // TODO guard against out of range (0-255)
  send[0] = '/';
  send[1] = '*';
  send[2] = '%';
  send[3] = '~';
  send[4] = lowByte(thr_i >> 8) & mask;
  send[5] = lowByte(thr_i & mask);
  send[6] = lowByte(yaw_i >> 8) & mask;
  send[7] = lowByte(yaw_i & mask);
  send[8] = lowByte(pit_i >> 8) & mask;
  send[9] = lowByte(pit_i & mask);
  send[10] = lowByte(rol_i >> 8) & mask;
  send[11] = lowByte(rol_i & mask);
  rfPrint(send);
}*/

void loop()
{
  thr = analogRead(1);
  yaw = analogRead(0);
  pitch = analogRead(3);
  roll = analogRead(2);
  serialPrint();
  //lcdPrint();

  int thr_i = convertRange(thr, thrMin, thrMax, 0, 1500); // TODO guard against out of range (0-255)
  int yaw_i = convertRange(yaw, yawMin, yawMax, 0, 1500); // TODO guard against out of range (0-255)
  int pit_i = convertRange(pitch, pitchMin, pitchMax, 0, 1500); // TODO guard against out of range (0-255)
  int rol_i = convertRange(roll, rollMin, rollMax, 0, 1500); // TODO guard against out of range (0-255)
  
  send[0] = '/';
  send[1] = '*';
  send[2] = '%';
  send[3] = '~';
  send[4] = lowByte(thr_i >> 8) & mask;
  send[5] = lowByte(thr_i & mask);
  send[6] = lowByte(yaw_i >> 8) & mask;
  send[7] = lowByte(yaw_i & mask);
  send[8] = lowByte(pit_i >> 8) & mask;
  send[9] = lowByte(pit_i & mask);
  send[10] = lowByte(rol_i >> 8) & mask;
  send[11] = lowByte(rol_i & mask);
  rfPrint(send);
}


