#include <Adafruit_LSM9DS1.h>

#include <Adafruit_Sensor_Set.h>
#include <Adafruit_Simple_AHRS.h>
#include <Madgwick.h>
#include <Mahony.h>

#include "radio.h"

const int MOTOR2 = 8; // B5 ORIGINALLY 1
const int MOTOR3 = 3; // E3 ORIGINALLY 2
const int MOTOR4 = 4; // E4 ORIGINALLY 3
const int MOTOR1 = 5; // E5 ORIGINALLY 4

//Ultimate motor values
float m1;
float m2;
float m3;
float m4;
float correction[4]; //the correction value for each motor
const float BOOST = 250.0;

//time variables
float t_prev = 0.0;
float t_curr = 0.0;

//orientation object
sensors_vec_t orientation;

//fields for main PID calc
float oldYaw = 0.0;
float targetYaw = 0.0;
float oldPitch = 0.0;
float targetPitch = 0.0;

//fields for yaw PID calc
float propY = 0.0;
float integY = 0.0;
float derivY = 0.0;
float yawPID = 0.0;

//fields for pitch PID calc
float propP = 0.0;
float integP = 0.0;
float derivP = 0.0;
float pitchPID = 0.0;
float pitch = 0.0;
float pitch_prev = 0.0;

float r_thr = 0.0;
float r_yaw = 0.0;
float r_pitch = 0.0;
float r_roll = 0.0;

typedef struct {
  int header;
  int thr;
  int yaw;
  int pitch;
  int roll;
} Control;
Control controls;

Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();
Adafruit_Simple_AHRS ahrs (&lsm.getAccel(), &lsm.getMag(), &lsm.getGyro());  

void configureLSM9DS1() {
  lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_2G);
  lsm.setupMag(lsm.LSM9DS1_MAGGAIN_4GAUSS);
  lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_245DPS);
}

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

  if(!lsm.begin()){
    while(1);
  }

  configureLSM9DS1(); 
}

void getRF()
{
  if(rfAvailable()) {
    char numRead = rfRead((uint8_t*)&controls, sizeof(Control)); //get values
    if(controls.header == 0xB3EF) {
      r_thr = controls.thr;
      r_yaw = controls.yaw;     
      r_pitch = controls.pitch;
      r_roll = controls.roll;
    }
  }
}

void calcYaw(float yaw, float rate){
  float err = yaw - targetYaw;
  propY = err;
  integY = (integY * 0.5) + err;
  derivY = rate;
  oldYaw = yaw;

  yawPID = 0 * propY + 0 * integY + 2 * derivY;
}

void calcPitch(float acc, float gyro){
  float lambda = 0.7;

  t_curr = millis();  
  pitch = (lambda) * (pitch_prev + ((t_curr - t_prev) / 1000) * gyro) + (1 - lambda) * (acc);
  float err = pitch - targetPitch;
  
  if(err > 1 + targetPitch || err < -1 + targetPitch) {
    propP = err;
    integP = (integP * 0.5) + err;
    derivP = (pitch - pitch_prev)/(t_curr - t_prev) - 60;

    pitchPID = 0.1 * propP + 0.1 * integP + 0.1 * derivP;
  }

  else {
    pitchPID = 0;
  }
  
  t_prev = t_curr;
  pitch_prev = pitch;
}


void PID(){
  if(ahrs.getQuad(&orientation)) {
    calcYaw(orientation.heading, orientation.g_z); //TODO: might need to convert g_z to degrees in library
    calcPitch(orientation.pitch, orientation.g_y); //TODO: might need to convert g_x to degrees in library

//    if(yawPID > 0) {
//      correction[0] 
//      correction[1]
//      correction[2]
//      correction[3]
//    }
//
//    else {
//      correction[0] 
//      correction[1]
//      correction[2]
//      correction[3]
//    }

      correction[0] = -pitchPID;
      correction[1] = -pitchPID;
      correction[2] = pitchPID;
      correction[3] = pitchPID;
  }
}

void mixer() {
  float v1 = correction[0] + r_thr/4;
  float v2 = correction[1] + r_thr/4;
  float v3 = correction[2] + r_thr/4 - BOOST;
  float v4 = correction[3] + r_thr/4 - BOOST;
  
  if(v1 >= 0 && v1 < 255.0)
    m1 = v1;
  else if(v1 > 255.0)
    m1 = 255.0;
  else
    m1 = 0.0;
  
  if(v2 >= 0 && v2 < 255.0)
    m2 = v2;
  else if(v2 > 255.0)
    m2 = 255.0;
  else
    m2 = 0.0;
    
  if(v3 >= 0 && v3 < 255.0)
    m3 = v3;
  else if(v3 > 255.0)
    m3 = 255.0;
  else
    m3 = 0.0;
  
  if(v4 >= 0 && v4 < 255.0)
    m4 = v4;
  else if(v4 > 255.0)
    m4 = 255.0;
  else
    m4 = 0.0;
}

void debug(){
  Serial.print(m1);
  Serial.print(", ");
  Serial.print(m2);
  Serial.print(", ");
  Serial.print(m3);
  Serial.print(", ");
  Serial.println(m4);

}

void loop()
{
  getRF();
  PID();
  mixer();
  debug();
  analogWrite(MOTOR1, m1);
  analogWrite(MOTOR2, m2);
  analogWrite(MOTOR3, m3);
  analogWrite(MOTOR4, m4);
}
