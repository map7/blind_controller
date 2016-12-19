/* 
   Curtain Automation
   Upload through emacs:
*/


#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61); 

// Define containts
#define LIGHT_PIN 0
#define LIGHT_THRESHOLD 800
#define DARK_THRESHOLD 400

#define TEMP_PIN 1
#define HOT_THRESHOLD 30
#define COLD_THRESHOLD 27
#define TEMP_VOLTAGE 5000

#define ONBOARD_LED 13
#define TRAVEL 1400

// Set variables
int curtain_state = 0; // 0 = closed, 1 = open
int light_status = 0;
double temp_status = 0;

boolean daylight = true;
boolean warm = false;

int up_btn = 2;

// Connect a stepper motor with 100 steps per revolution (1.8 degree)
// to motor port #2 (M3 and M4)
Adafruit_StepperMotor *myMotor = AFMS.getStepper(100, 2);

void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps
  Serial.println("Setting up Curtain Automation...");

  AFMS.begin();  // create with the default frequency 1.6KHz
  //AFMS.begin(1000);  // OR with a different frequency, say 1KHz

  // Set stepper motor rotation
  myMotor->setSpeed(30);

  // Setup button
  pinMode(up_btn, INPUT);
  
  // Initialize motor
  delay(1000);
}

void Curtain(boolean curtain_state) {
  digitalWrite(ONBOARD_LED, curtain_state ? HIGH : LOW);

  if (curtain_state){
    Serial.println("Opening curtain...");
    // Try SINGLE, DOUBLE, INTERLEAVE or MICROSTOP
    myMotor->step(TRAVEL, BACKWARD, DOUBLE);
    myMotor->release();
  }else{
    Serial.println("Closing curtain...");
    myMotor->step(TRAVEL, FORWARD, DOUBLE);
    myMotor->release();
  }
}

void loop() {

  // poll photocell value
  light_status = analogRead(LIGHT_PIN);
  delay(500);

  // print light_status value to Serial port
  Serial.print("Photocell value = ");
  Serial.println(light_status);
  Serial.println("");

  // poll temperature
  int temp_reading = analogRead(TEMP_PIN);
  delay(200);
 
  // DON'T do this ever, it's wrong, I don't know why!!!!
  /* float voltage = temp_reading * (TEMP_VOLTAGE / 1024.0); */
  
  /* eg: 765mV - 500mV / 10 = 26.5 could be right */
  /* http://www.instructables.com/id/Temperature-Sensor-Tutorial/ */
  float temp_voltage = temp_reading * 0.004882814;
  //float temp_Celsius = (temp_reading - 500) / 10.0; //Wrong
  float temp_Celsius = (temp_voltage - 0.5) * 100.0;  
  float temp_Fahrenheit = (temp_Celsius * 9 / 5) + 32;

  /* // print temp_status value to the serial port */
  /* Serial.print("Temperature reading (RAW)  = "); */
  /* Serial.println(temp_reading); */

  /* Serial.print("Temperature voltage = "); */
  /* Serial.println(temp_voltage); */
  
  Serial.print("Temperature value (Celsius)  = ");
  Serial.println(temp_Celsius);
  /* Serial.print("Temperature value (Fahrenheit) = "); */
  /* Serial.println(temp_Fahrenheit); */
  Serial.println("");


  switch (curtain_state){
  case 0: // Currently Closed
    if (light_status > LIGHT_THRESHOLD && temp_Celsius < COLD_THRESHOLD){
      Serial.println("It's daytime and cold inside, open curtain");
      curtain_state = 1;
      Curtain(curtain_state);      
    }
  
    break;

  case 1: // Currently Open
    if (light_status < DARK_THRESHOLD){
      Serial.println("night time, close curtain");
      curtain_state = 0;
      Curtain(curtain_state);
    }

    if (light_status > LIGHT_THRESHOLD && temp_Celsius > HOT_THRESHOLD){
      Serial.println("It's hot outside and in, close curtain");
      curtain_state = 0;
      Curtain(curtain_state);      
    }    
  
    break;
  }


/*
  if (light_status > LIGHT_THRESHOLD){
    Serial.println("Daylight true");
    daylight = true;
  } else{
    Serial.println("Daylight false");
    daylight = false;
  }
  
  if (temp_Celsius > TEMP_THRESHOLD) {
    Serial.println("Warm true");
    warm = true;
  } else {
    Serial.println("Warm false");
    warm = false;
  }
 
  switch (curtain_state){
  case 0:
    // if we have light and it's cold inside, open curtain
    if (daylight && !warm){
      curtain_state = 1;
      Curtain(curtain_state);
    }
    break;

  case 1:
    // if it's night and it is warm, close curtain
    if (!daylight && warm){
      curtain_state = 0;
      Curtain(curtain_state);
    }
    break;
  }
*/

  /* Test button */
  if (digitalRead(up_btn) == HIGH){
    if (curtain_state == 0){
      Serial.println("Open...");
      myMotor->step(TRAVEL, BACKWARD, DOUBLE);
      myMotor->release();
      curtain_state = 1;
    }else{
      Serial.println("Close...");
      myMotor->step(TRAVEL, FORWARD, DOUBLE);  
      myMotor->release();
      curtain_state = 0;
    }
  }
}
