#include <serLCD.h>
#include "radio.h"

#define PIN_YAW       0
#define PIN_THROTTLE  1
#define PIN_ROLL      2
#define PIN_PITCH     3
#define PIN_POT1      7
#define PIN_POT2      6
#define PIN_BTN1      16
#define PIN_BTN2      17

#define YAW   0
#define THR   1
#define ROLL  2
#define PITCH 3

const int minR[] = {0, 0, 0, 0};             // yaw, throttle, roll, pitch min range
const int maxR[] = {1023, 1500, 1023, 1023}; // yaw, throttle, roll, pitch max range
const int minG[] = {158, 151, 117, 163};     // yaw, throttle, roll, pitch gimbal min
const int maxG[] = {817, 815, 816, 816};     // yaw, throttle, roll, pitch gimbal max

int yaw = 0;
int thr = 0;
int roll = 0;
int pitch = 0;
float pot1 = 0.0;
float pot2 = 0.0;
int but1 = 0;
int but2 = 0;

typedef struct {
  int header;
  int yaw;
  int thr;
  int roll;
  int pitch;
  float pot1;
  float pot2;
  int but1;
  int but2;
} Control;

serLCD lcd;
Control controls;

int* gimbalsRaw[] = {&yaw, &thr, &roll, &pitch};
int* gimbals[] = {&controls.yaw, &controls.thr, &controls.roll, &controls.pitch};

void serialPrint()
{
  Serial.print("Raw:\t");
  for(int i = 0; i < 4; i++) {
    Serial.print(*(gimbalsRaw[i]));
    Serial.print(" ");
  }
  Serial.print(pot1);
  Serial.print(" ");
  Serial.print(pot2);
  Serial.print(" ");
  Serial.print(but1);
  Serial.print(" ");
  Serial.print(but2);
  Serial.print("\t\tScaled:\t");
  for(int i = 0; i < 4; i++) {
    Serial.print(*(gimbals[i]));
    Serial.print(" ");
  }
  Serial.print(" ");
  Serial.print(controls.pot1);
  Serial.print(" ");
  Serial.print(controls.pot2);
  Serial.print(" ");
  Serial.print(controls.but1);
  Serial.print(" ");
  Serial.println(controls.but2);
}

void lcdPrint()
{
  lcd.clear();
  lcd.selectLine(0);
  lcd.print(controls.yaw);
  lcd.print(" ");
  lcd.print(controls.thr);
  lcd.print("  ");
  lcd.print(controls.pot1 * 100);
  lcd.print("  ");
  lcd.print(controls.but1);
  lcd.selectLine(1);
  lcd.print(controls.roll);
  lcd.print(" ");
  lcd.print(controls.pitch);
  lcd.print("  ");
  lcd.print(controls.pot2 * 100);
  lcd.print("  ");
  lcd.print(controls.but2);
  delay(100);
}

int convertRange(long value, long oldMin, long oldMax, long newMin, long newMax)
{
  return ((value - oldMin) * (newMax - newMin)) / ((oldMax - oldMin) + newMin);
}

float potRange(long value, long oldMin, long oldMax, long newMin, long newMax)
{
  float result = abs((float)((value - oldMin) * (newMax - newMin)) / (float)((oldMax - oldMin) + newMin));
  if (result < newMin) return newMin;
  if (result > newMax) return newMax;
  return result;
}

void fixGimbals() {
  for(int i = 0; i < 4; i++) {
    if(*(gimbals[i]) < minR[i])
      *(gimbals[i]) = minR[i];
    else if(*(gimbals[i]) > maxR[i])
      *(gimbals[i]) = maxR[i];
  }
}

void setup()
{
  Serial.begin(9600);
  
  pinMode(PIN_YAW, INPUT);
  pinMode(PIN_THROTTLE, INPUT);
  pinMode(PIN_ROLL, INPUT);
  pinMode(PIN_PITCH, INPUT);
  pinMode(PIN_POT1, INPUT);
  pinMode(PIN_POT2, INPUT);
  pinMode(PIN_BTN1, INPUT_PULLUP);
  pinMode(PIN_BTN2, INPUT_PULLUP);

  rfBegin(19);
}

void loop()
{

  yaw = analogRead(PIN_YAW);
  thr = analogRead(PIN_THROTTLE);
  roll = analogRead(PIN_ROLL);
  pitch = analogRead(PIN_PITCH);
  pot1 = analogRead(PIN_POT1);
  pot2 = analogRead(PIN_POT2);
  but1 = digitalRead(PIN_BTN1);
  but2 = digitalRead(PIN_BTN2);

  serialPrint();
  lcdPrint();

  controls.header = 0xB3EF;
  controls.yaw = convertRange(yaw, minG[YAW], maxG[YAW], minR[YAW], maxR[YAW]);
  controls.thr = convertRange(thr, minG[THR], maxG[THR], minR[THR], maxR[THR]);
  controls.roll = convertRange(roll, minG[ROLL], maxG[ROLL], minR[ROLL], maxR[ROLL]);
  controls.pitch = convertRange(pitch, minG[PITCH], maxG[PITCH], minR[PITCH], maxR[PITCH]);
  controls.pot1 = potRange(pot1, 111, 816, 0, 4);
  controls.pot2 = potRange(pot2, 111, 816, 0, 4);
  controls.but1 = (but1 == 0 ? 1 : 0);
  controls.but2 = (but2 == 0 ? 1 : 0);

  fixGimbals();

  rfWrite((uint8_t*)&controls, sizeof(Control));
}
