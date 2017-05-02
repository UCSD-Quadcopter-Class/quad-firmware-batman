#include "radio.h"

const int MOTOR1 = 8; // B5
const int MOTOR2 = 3; // E3
const int MOTOR3 = 4; // E4
const int MOTOR4 = 5; // E5

int thr = 0;
int yaw = 0;
int pitch = 0;
int roll = 0;

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
  pinMode(MOTOR1, OUTPUT);
  pinMode(MOTOR2, OUTPUT);
  pinMode(MOTOR3, OUTPUT);
  pinMode(MOTOR4, OUTPUT);
  analogWrite(MOTOR1, 0);
  analogWrite(MOTOR2, 0);
  analogWrite(MOTOR3, 0);
  analogWrite(MOTOR4, 0);
}

void getRF()
{
  if(rfAvailable()) {
    char numRead = rfRead((uint8_t*)&controls, sizeof(Control)); //get values
    if(controls.header == 0xB3EF) {
      thr = controls.thr;
      Serial.print(thr);
      Serial.print(", ");
      yaw = controls.yaw;
      Serial.print(yaw);
      Serial.print(", ");      
      pitch = controls.pitch;
      Serial.print(pitch);
      Serial.print(", "); 
      roll = controls.roll;
      Serial.println(roll); 
    }
  }
}

void loop()
{
  getRF();
  analogWrite(MOTOR1, thr);
  analogWrite(MOTOR2, thr);
  analogWrite(MOTOR3, thr);
  analogWrite(MOTOR4, thr);
}
