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
float pCorrect[4]; //the correction value for each motor
float rCorrect[4]; //the correction value for each motor
float yCorrect[4]; //the correction value for each motor
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
float oldRoll = 0.0;
float targetRoll = 0.0;

//fields for pitch PID calc
float propP = 0.0;
float integP = 0.0;
float derivP = 0.0;
float pitchPID = 0.0;
float pitch = 0.0;
float pitch_prev = 0.0;

//fields for pitch PID calc
float propR = 0.0;
float integR = 0.0;
float derivR = 0.0;
float rollPID = 0.0;
float roll = 0.0;
float roll_prev = 0.0;

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
      rf.yaw = controls.yaw * 0.08789;  
      rf.thr = controls.thr;   
      rf.roll = controls.roll * 0.08789;
      rf.pitch = controls.pitch * 0.08789;
      rf.pot1 = controls.pot1;
      rf.pot2 = controls.pot2;
      rf.but1 = controls.but1;
      rf.but2 = controls.but2;
    }
  }
}

void calcYaw(){
  yawPID = 0.04 * rf.yaw;
}

void calcRoll(float acc, float gyro) {
  float lambda = 0.8;
  targetRoll = rf.roll;

  t_curr = millis();  
  roll = ((lambda) * (roll + ((t_curr - t_prev) / 1000.0) * -gyro) + (1 - lambda) * (acc));
  float err = roll - targetRoll;
  
  propR = err;
  integR = (integR * 0.75) + err;
  derivR = (roll - roll_prev) * 100.0/(t_curr - t_prev); // extremely noisy, need to filter

  if(integR >= 100.0) integR = 100.0;
  if(integR <= -100.0) integR = -100.0;

  pitchPID = (0.67) * propP + (0.19) * integP + (3.46) * derivP; //Deriv: 2.65

  roll_prev = roll;
  t_prev = t_curr;
}

void calcPitch(float acc, float gyro){
  float lambda = 0.8;
  targetPitch = rf.pitch;
  

  t_curr = millis();  
  pitch = ((lambda) * (pitch + ((t_curr - t_prev) / 1000.0) * -gyro) + (1 - lambda) * (acc));
  float err = pitch - targetPitch;
  
  propP = err;
  integP = (integP * 0.75) + err;
  derivP = (pitch - pitch_prev) * 100.0/(t_curr - t_prev); // extremely noisy, need to filter

  if(integP >= 100.0) integP = 100.0;
  if(integP <= -100.0) integP = -100.0;

  pitchPID = (0.67) * propP + (0.19) * integP + (3.46) * derivP; //Deriv: 2.65
//  pitchPID = (rf.pot1) * propP + (0.0) * integP + (rf.pot2) * derivP; //Deriv: 2.65

  pitch_prev = pitch;
  t_prev = t_curr;
}

void PID(){
  if(ahrs.getQuad(&orientation)) {
    calcYaw();
    calcPitch(orientation.pitch, orientation.g_y);
    calcRoll(orientation.roll, orientation.g_x);

    yCorrect[0] = -yawPID;
    yCorrect[1] = yawPID;
    yCorrect[2] = -yawPID;
    yCorrect[3] = yawPID;

    rCorrect[0] = -rollPID;
    rCorrect[1] = rollPID;
    rCorrect[2] = rollPID;
    rCorrect[3] = -rollPID;

    pCorrect[0] = -pitchPID; // might need to change - to + and + to minus for rollPID
    pCorrect[1] = -pitchPID;
    pCorrect[2] = pitchPID;
    pCorrect[3] = pitchPID;
  }
}

void mixer() {
  float SHELF = 200;
  float TRIM = 0.61;
  float v1 = (pCorrect[0] + rf.thr/4);
  float v2 = (pCorrect[1] + rf.thr/4);
  float v3 = (pCorrect[2] + rf.thr/4) * TRIM;
  float v4 = (pCorrect[3] + rf.thr/4) * TRIM;

  if(v3 > SHELF) v3 = SHELF;
  if(v4 > SHELF) v4 = SHELF;
  
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
  Serial.print(pitch);
  Serial.print(" ");
  Serial.print(rf.pot1);
  Serial.print(" ");
  Serial.print(rf.pot2);
  Serial.print(" ");
  Serial.print('\n');

//  Serial.print(orientation.pitch);
//  Serial.print(" ");
//  Serial.print(pitch);
//  Serial.print(" ");
//  Serial.print(propP);
//  Serial.print(" ");
//  Serial.print(derivP);
//  Serial.print(" ");
//  Serial.print(integP);
//  Serial.print(" ");
//  Serial.print("rf.pitch: ");
//  Serial.print(rf.pitch);
//  Serial.print("\n");
}

void loop()
{
  getRF();
  PID();
  mixer();
  debug();
  if(rf.but1)
    motorsOff = true;
  if(rf.but2)
    motorsOff = false;
  analogWrite(MOTOR1, (motorsOff ? 0 : m1));
  analogWrite(MOTOR2, (motorsOff ? 0 : m2));
  analogWrite(MOTOR3, (motorsOff ? 0 : m3));
  analogWrite(MOTOR4, (motorsOff ? 0 : m4));
}
