#include <Adafruit_LSM9DS1.h>

#include <Adafruit_Sensor_Set.h>
#include <Adafruit_Simple_AHRS.h>
#include <Madgwick.h>
#include <Mahony.h>

#include "radio.h"

#define MOTOR1 5 // E5 ORIGINALLY 4
#define MOTOR2 8 // B5 ORIGINALLY 1
#define MOTOR3 3 // E3 ORIGINALLY 2
#define MOTOR4 4 // E4 ORIGINALLY 3

//Ultimate motor values
float m1;
float m2;
float m3;
float m4;
float correction[4]; //the correction value for each motor
float TRIM = 0.66;
bool motorsOff = false;

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

Control controls;
Control rf;

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
      rf.yaw = controls.yaw;  
      rf.thr = controls.thr;   
      rf.roll = controls.roll;
      rf.pitch = controls.pitch;
      rf.pot1 = controls.pot1;
      rf.pot2 = controls.pot2;
      rf.but1 = controls.but1;
      rf.but2 = controls.but2;
    }
  }
}

void calcYaw(float yaw, float rate){
  float err = yaw - targetYaw;
  propY = err;
  integY = (integY * 0.5) + err;
  derivY = rate;
  oldYaw = yaw;

  yawPID = 0 * propY + 0 * integY + 0 * derivY;
}

void calcPitch(float acc, float gyro){
  float lambda = 0.8;

  t_curr = millis();  
  pitch = ((lambda) * (pitch + ((t_curr - t_prev) / 1000) * -gyro) + (1 - lambda) * (acc));
  float err = pitch - targetPitch;
  
//  if(err > targetPitch || err < targetPitch) {
    propP = err;
    integP = (integP * 0.5) + err;
    derivP = (pitch - pitch_prev)/((t_curr - t_prev)/100); // extremely noisy, need to filter

    if(integP >= 100.0) integP = 100.0;
    if(integP <= -100.0) integP = -100.0;

    pitchPID = (rf.pot1) * propP + (0.0) * integP + (rf.pot2) * derivP;
//  }
//
//  else {
//    pitchPID = 0;
//  }
  pitch_prev = pitch;
  t_prev = t_curr;

  //Serial.print(orientation.pitch);
  //Serial.print(" ");
  Serial.print(pitch);
  Serial.print(" ");
  //Serial.print(((t_curr - t_prev) / 1000) * -gyro);
  //Serial.print(" ");
  //Serial.print((orientation.g_y)/5);
  //Serial.print(" ");
  Serial.print((rf.pot1) * propP + (0.0) * integP + (rf.pot2) * derivP);
  Serial.print(" ");
  Serial.print(propP);
  Serial.print(" ");
  Serial.print(derivP);
  Serial.print(" ");
  Serial.print("\n");
  
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
  float v1 = correction[0] + rf.thr/4;
  float v2 = correction[1] + rf.thr/4;
  float v3 = (correction[2] + rf.thr/4) * TRIM;
  float v4 = (correction[3] + rf.thr/4) * TRIM;
  
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
  Serial.print(propP * rf.pot1);
  Serial.print(" ");
  Serial.print(derivP * rf.pot2);
  Serial.print(" ");
  Serial.print(rf.pot1);
  Serial.print(" ");
  Serial.print(rf.pot2);
  Serial.print(" ");
  Serial.print('\n');
}

void loop()
{
  getRF();
  PID();
  mixer();
  //debug();
  if(rf.but1)
    motorsOff = true;
  if(rf.but2)
    motorsOff = false;
  analogWrite(MOTOR1, (motorsOff ? 0 : m1));
  analogWrite(MOTOR2, (motorsOff ? 0 : m2));
  analogWrite(MOTOR3, (motorsOff ? 0 : m3));
  analogWrite(MOTOR4, (motorsOff ? 0 : m4));
}
