#include <Adafruit_LSM9DS1.h>

#include <Adafruit_Sensor_Set.h>
#include <Adafruit_Simple_AHRS.h>
#include <Madgwick.h>
#include <Mahony.h>

#include "radio.h"

const int MOTOR1 = 8; // B5
const int MOTOR2 = 3; // E3
const int MOTOR3 = 4; // E4
const int MOTOR4 = 5; // E5

float thr = 0;
float yaw = 0;
float pitch = 0;
float roll = 0;

typedef struct {
  int header;
  int thr;
  int yaw;
  int pitch;
  int roll;
} Control;
Control controls;

sensors_vec_t *orientation;
sensor_t acc;
sensor_t mag;
Adafruit_LSM9DS1::Sensor *accelerometer;
Adafruit_LSM9DS1::Sensor *magnetometer;
Adafruit_Simple_AHRS ahrs (accelerometer, magnetometer);  

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

  acc.type = SENSOR_TYPE_ACCELEROMETER; //TODO: USE ENUM????
  accelerometer->getSensor(&acc);

  mag.type = SENSOR_TYPE_MAGNETIC_FIELD;
  magnetometer->getSensor(&mag);
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

void PID(){
  ahrs.getOrientation(orientation);
  Serial.println(orientation->roll);
  Serial.println(orientation->pitch);
  Serial.println(orientation->heading);
  
    
}

void loop()
{
  getRF();
  PID();
  analogWrite(MOTOR1, thr);
  analogWrite(MOTOR2, thr);
  analogWrite(MOTOR3, thr);
  analogWrite(MOTOR4, thr);
}
