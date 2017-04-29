#include "radio.h"

const int MOTOR1 = 8; // B5
const int MOTOR2 = 3; // E3
const int MOTOR3 = 4; // E4
const int MOTOR4 = 5; // E5

int thr = 0;
int yaw = 0;
int pitch = 0;
int roll = 0;
int rfThrot = 0;
int sThrot = 0;

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

void loop()
{
  /*if (rfAvailable())
  {
    unsigned char tmp = rfRead();
    Serial.println(int (tmp)); // issue: reads a byte...
    if(tmp >= 0 && tmp <= 255) { // make sure received value is in range
      thr = tmp;
    } else {
      Serial.print("Not in range: ");
      Serial.println(tmp);
      thr = 0;
    }
  }*/

  /*if (rfAvailable()) {
    //unsigned char tmp = rfRead();
    //Serial.println((unsigned char)tmp);
    switch (unsigned char tmp = rfRead()) {
      case 252:
        Serial.print("Received 252, immediately available value: ");
        if (rfAvailable()) {
          tmp = rfRead();
          Serial.print(tmp);
          thr = tmp;
        }
        Serial.print("\n");
        break;
      case 253:
        Serial.print("Received 253, immediately available value:");
        if (rfAvailable()) {
          tmp = rfRead();
          Serial.print(tmp);
          yaw = tmp;
        }
        Serial.print("\n");
        break;
      case 254:
        Serial.print("Received 254, immediately available value:");
        if (rfAvailable()) {
          tmp = rfRead();
          Serial.print(tmp);
          pitch = tmp;
        }
        Serial.print("\n");
        break;
      case 255:
        Serial.print("Received 255, immediately available value:");
        if (rfAvailable()) {
          tmp = rfRead();
          Serial.print(tmp);
          roll = tmp;
        }
        Serial.print("\n");
        break;
      default:
        Serial.print("Unknown flag value: ");
        Serial.println(tmp);
        delay(1);
    }
  }*/

  //Serial.print("Throttle: ");
  //Serial.println(throttle);
  thr = 0;
  analogWrite(MOTOR1, thr);
  analogWrite(MOTOR2, thr);
  analogWrite(MOTOR3, thr);
  analogWrite(MOTOR4, thr);
}

