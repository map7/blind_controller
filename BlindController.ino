/* 
   Curtain Automation
   Compile through emacs: M-x compile make -k
   Upload through emacs:  M-x compile make -k upload
*/

#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* CS=*/ 10, /* reset=*/ 8);

#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61); 

// Define containts
#define LIGHT_PIN 0
#define LIGHT_THRESHOLD 700
#define DARK_THRESHOLD 400

#define TEMP_PIN 1
#define HOT_THRESHOLD 27
#define COLD_THRESHOLD 25
#define TEMP_VOLTAGE 5000

#define ONBOARD_LED 13
#define TRAVEL 1400

// Set variables
int curtain_state = 0; // 0 = closed, 1 = open
int light_status = 0;
char light_status_str[3];

double temp_status = 0;
int temp_reading = 0;
char temp_reading_str[3];

boolean daylight = true;
boolean warm = false;

int up_btn = 2;

// Connect a stepper motor with 100 steps per revolution (1.8 degree)
// to motor port #2 (M3 and M4)
Adafruit_StepperMotor *myMotor = AFMS.getStepper(100, 2);

void setup() {
  u8g2.begin();
  
  /* Serial.begin(9600);           // set up Serial library at 9600 bps */
  /* Serial.println("Setting up Curtain Automation..."); */

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
    /* Serial.println("Opening curtain..."); */
    // Try SINGLE, DOUBLE, INTERLEAVE or MICROSTOP
    myMotor->step(TRAVEL, BACKWARD, DOUBLE);
    myMotor->release();
  }else{
    /* Serial.println("Closing curtain..."); */
    myMotor->step(TRAVEL, FORWARD, DOUBLE);
    myMotor->release();
  }
}

void loop() {
  // Get light value and convert to a char array
  light_status = analogRead(LIGHT_PIN);
  sprintf(light_status_str, "%03i", light_status);

  // Get temperature
  temp_reading = analogRead(TEMP_PIN);
  float temp_voltage = temp_reading * 0.004882814;
  float temp_Celsius = (temp_voltage - 0.5) * 100.0;  
  float temp_Fahrenheit = (temp_Celsius * 9 / 5) + 32;
  dtostrf(temp_Celsius, 2, 0, temp_reading_str);

  // Display to LCD
  u8g2.clearBuffer();               // clear the internal memory

  u8g2.setDisplayRotation(U8G2_R2); // 180 degrees rotate
  u8g2.setFlipMode(1);          // hardware flip

  u8g2.setFont(u8g2_font_osr26_tn);	// choose font
  u8g2.drawStr(0,45,"00:00");       // write text to memory

  u8g2.setFont(u8g2_font_7x13B_tf);	// choose font

  u8g2.drawStr(93,10,"Light");
  u8g2.drawStr(106,23,light_status_str); 
  
  u8g2.drawStr(106,48,"TMP"); 
  u8g2.drawStr(106,61,temp_reading_str); 

  u8g2.setFont(u8g2_font_unifont_t_symbols);
  u8g2.drawUTF8(94, 49, "☔");      // UTF8 characters ☀ ☁ ☂ ☔ 
  
  // transfer mem to display
  u8g2.sendBuffer();               
  delay(1000);

  // Blind control
  switch (curtain_state){
  case 0: // Currently Closed
    if (light_status > LIGHT_THRESHOLD && temp_Celsius < COLD_THRESHOLD){
      /* Serial.println("It's daytime and cold inside, open curtain"); */
      curtain_state = 1;
      Curtain(curtain_state);      
    }
  
    break;

  case 1: // Currently Open
    if (light_status < DARK_THRESHOLD){
      /* Serial.println("night time, close curtain"); */
      curtain_state = 0;
      Curtain(curtain_state);
    }

    if (light_status > LIGHT_THRESHOLD && temp_Celsius > HOT_THRESHOLD){
      /* Serial.println("It's hot outside and in, close curtain"); */
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

  /* /\* Test button *\/ */
  /* if (digitalRead(up_btn) == HIGH){ */
  /*   if (curtain_state == 0){ */
  /*     /\* Serial.println("Open..."); *\/ */
  /*     myMotor->step(TRAVEL, BACKWARD, DOUBLE); */
  /*     myMotor->release(); */
  /*     curtain_state = 1; */
  /*   }else{ */
  /*     /\* Serial.println("Close..."); *\/ */
  /*     myMotor->step(TRAVEL, FORWARD, DOUBLE);   */
  /*     myMotor->release(); */
  /*     curtain_state = 0; */
  /*   } */
  /* } */
}
