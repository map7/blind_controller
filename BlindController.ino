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

#define TRAVEL 1400

// Set variables
int curtain_state = 0; // 0 = closed, 1 = open

// Light variables
int light_status = 0;
int light_status_int = 0;
char light_status_str[3];

// Temperature variables
float temp_Celsius;
float temp_Fahrenheit;
double temp_status = 0;
int temp_reading = 0;
char temp_reading_str[3];
boolean daylight = true;
boolean warm = false;

// Buttons & LEDS
int buttonPins[] = {2,3,4};
int ledPins[] = {5,6,7};
int auto_curtain = 0;  // 0 = manual, 1 = auto

// Connect a stepper motor with 100 steps per revolution (1.8 degree)
// to motor port #2 (M3 and M4)
Adafruit_StepperMotor *myMotor = AFMS.getStepper(100, 2);

void AllLedsOn(){
  for(int thisLed=0; thisLed<3; thisLed++){
    digitalWrite(ledPins[thisLed], HIGH);
  }
}

// Reset all LEDs to low
void AllLedsOff(){
  for(int thisLed=0; thisLed<3; thisLed++){
    digitalWrite(ledPins[thisLed], LOW);
  }
}

void FlashLeds(int times) {
  for(int current=0; current<times; current++){
    AllLedsOn();
    delay(150);
    AllLedsOff();
    delay(150);
  }
}

void Curtain(boolean curtain_state) {
  if (curtain_state){
    digitalWrite(ledPins[0], HIGH);
    digitalWrite(ledPins[1], LOW);

    // Delay maybe could help freezing
    delay(1000);
    
    /* Serial.println("Opening curtain..."); */
    // Try SINGLE, DOUBLE, INTERLEAVE or MICROSTOP
    myMotor->step(TRAVEL, BACKWARD, DOUBLE);
    myMotor->release();
  }else{
    digitalWrite(ledPins[0], LOW);
    digitalWrite(ledPins[1], HIGH);

    // Delay maybe could help freezing
    delay(1000);
    
    /* Serial.println("Closing curtain..."); */
    myMotor->step(TRAVEL, FORWARD, DOUBLE);
    myMotor->release();
  }
}

void SetupButtons(){
  // Setup buttons
  for(int thisBtn=0; thisBtn<3; thisBtn++){
    pinMode(buttonPins[thisBtn], INPUT);
  }
}

void ButtonActions(){

  // Open
  if (digitalRead(buttonPins[0]) == HIGH){
    curtain_state = 1;
    Curtain(curtain_state);    
  }

  // Close
  if (digitalRead(buttonPins[1]) == HIGH){
    curtain_state = 0;
    Curtain(curtain_state);    
  }

  // Auto
  if (digitalRead(buttonPins[2]) == HIGH){
    if (auto_curtain == 1){
      auto_curtain = 0;
      digitalWrite(ledPins[2], LOW);
    }else{
      auto_curtain = 1;
      digitalWrite(ledPins[2], HIGH);
    }
  }  
}

void SetupLEDs(){
  // Setup LEDs
  for(int thisLed=0; thisLed<3; thisLed++){
    pinMode(ledPins[thisLed], OUTPUT);
    digitalWrite(ledPins[thisLed], HIGH);
    delay(300);
  }

  // Do a little dance
  FlashLeds(3);
  delay(300);
}

void SetupMotor(){
  AFMS.begin();  // create with the default frequency 1.6KHz
  //AFMS.begin(1000);  // OR with a different frequency, say 1KHz

  // Set stepper motor rotation
  myMotor->setSpeed(30);

  // Initialize motor
  delay(1000);
}

void SetupSerial(){
  // Setup serial
  Serial.begin(9600);           // set up Serial library at 9600 bps
  Serial.println("Setting up Curtain Automation...");
}

void setup() {
  SetupLEDs();                  /* LED */
  SetupButtons();               /* Buttons */  
  u8g2.begin();                 /* LCD */
  SetupSerial();                /* Serial */
  SetupMotor();                 /* Motor */
  digitalWrite(ledPins[1], HIGH); /* Show status Closed */
}

void GetLight(){
  // Get light value and convert to a char array
  light_status = analogRead(LIGHT_PIN);
  light_status_int = light_status;
  sprintf(light_status_str, "%03i", light_status_int);
}

void GetTemperature(){
  // Get temperature
  temp_reading = analogRead(TEMP_PIN);
  float temp_voltage = temp_reading * 0.004882814;
  temp_Celsius = (temp_voltage - 0.5) * 100.0;  
  temp_Fahrenheit = (temp_Celsius * 9 / 5) + 32;
  dtostrf(temp_Celsius, 2, 0, temp_reading_str);
}

// Display to LCD
void DisplayInfo(){
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
  
  // Display light_status & temp_Celsius in Serial
  Serial.println(light_status);
  Serial.println(temp_Celsius);
}

void loop() {

  GetLight();
  ButtonActions();
  GetTemperature();
  DisplayInfo();

  if (auto_curtain == 1){
    // Blind control
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
  }
  
  /* // Show aliveness */
  /* digitalWrite(ledPins[2], HIGH); */
  /* delay(500); */
  /* digitalWrite(ledPins[2], LOW); */
  
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
