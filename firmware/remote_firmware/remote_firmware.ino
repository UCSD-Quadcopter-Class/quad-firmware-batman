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

const int minR[] = {0, 0, 0, 0};             // yaw, throttle, roll, pitch min range for conversion
const int maxR[] = {1023, 1500, 1023, 1023}; // yaw, throttle, roll, pitch max range for conversion

// values from remote
struct {
  int yaw = 0;
  int thr = 0;
  int roll = 0;
  int pitch = 0;
  float pot1 = 0.0;
  float pot2 = 0.0;
  int but1 = 0;
  int but2 = 0;
  const int Min[4] = {158, 151, 117, 163}; // yaw, throttle, roll, pitch gimbal min
  const int Max[4] = {817, 815, 816, 816}; // yaw, throttle, roll, pitch gimbal max
} r;

// struct to be sent to quadcopter
struct {
  int header;
  int yaw;
  int thr;
  int roll;
  int pitch;
  float pot1;
  float pot2;
  int but1;
  int but2;
} q;

serLCD lcd;

int* gimbalsRaw[] = {&r.yaw, &r.thr, &r.roll, &r.pitch};
int* gimbals[] = {&q.yaw, &q.thr, &q.roll, &q.pitch};

void serialPrint()
{
  Serial.print("Raw:\t");
  for (int i = 0; i < 4; i++) {
    Serial.print(*(gimbalsRaw[i]));
    Serial.print(" ");
  }
  Serial.print(r.pot1);
  Serial.print(" ");
  Serial.print(r.pot2);
  Serial.print(" ");
  Serial.print(r.but1);
  Serial.print(" ");
  Serial.print(r.but2);
  Serial.print("\t\tScaled:\t");
  for (int i = 0; i < 4; i++) {
    Serial.print(*(gimbals[i]));
    Serial.print(" ");
  }
  Serial.print(" ");
  Serial.print(q.pot1);
  Serial.print(" ");
  Serial.print(q.pot2);
  Serial.print(" ");
  Serial.print(q.but1);
  Serial.print(" ");
  Serial.println(q.but2);
}

void lcdPrint()
{
  lcd.clear();
  lcd.selectLine(0);
  lcd.print(q.yaw);
  lcd.print(" ");
  lcd.print(q.thr);
  lcd.print("  ");
  lcd.print(q.pot1 * 100);
  lcd.print("  ");
  lcd.print(q.but1);
  lcd.selectLine(1);
  lcd.print(q.roll);
  lcd.print(" ");
  lcd.print(q.pitch);
  lcd.print("  ");
  lcd.print(q.pot2 * 100);
  lcd.print("  ");
  lcd.print(q.but2);
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
  for (int i = 0; i < 4; i++) {
    if (*(gimbals[i]) < minR[i])
      *(gimbals[i]) = minR[i];
    else if (*(gimbals[i]) > maxR[i])
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

  r.yaw = analogRead(PIN_YAW);
  r.thr = analogRead(PIN_THROTTLE);
  r.roll = analogRead(PIN_ROLL);
  r.pitch = analogRead(PIN_PITCH);
  r.pot1 = analogRead(PIN_POT1);
  r.pot2 = analogRead(PIN_POT2);
  r.but1 = digitalRead(PIN_BTN1);
  r.but2 = digitalRead(PIN_BTN2);

  serialPrint();
  lcdPrint();

  q.header = 0xB3EF;
  q.yaw = convertRange(r.yaw, r.Min[YAW], r.Max[YAW], minR[YAW], maxR[YAW]);
  q.thr = convertRange(r.thr, r.Min[THR], r.Max[THR], minR[THR], maxR[THR]);
  q.roll = convertRange(r.roll, r.Min[ROLL], r.Max[ROLL], minR[ROLL], maxR[ROLL]);
  q.pitch = convertRange(r.pitch, r.Min[PITCH], r.Max[PITCH], minR[PITCH], maxR[PITCH]);
  q.pot1 = potRange(r.pot1, 111, 816, 0, 4);
  q.pot2 = potRange(r.pot2, 111, 816, 0, 4);
  q.but1 = (r.but1 == 0 ? 1 : 0);
  q.but2 = (r.but2 == 0 ? 1 : 0);

  fixGimbals();

  rfWrite((uint8_t*)&q, sizeof(q));
}
