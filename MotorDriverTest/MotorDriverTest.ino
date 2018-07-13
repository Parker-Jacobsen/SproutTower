#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "Arduino.h"
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

// motor one
int in1 = 4;
int in2 = 5;

//Motor two
int in3=6;
int in4=7;

void setup()
{
  // set all the motor control pins to outputs
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  
  Serial.begin(9600);           // set up Serial library at 9600 bps

   // activate LCD module
  lcd.begin ();
  lcd.backlight();        //Turn backlight on
  lcd.home();
}
void loop()
{
  lcd.clear();
  lcd.home();
  lcd.print("M1 forward");
  digitalWrite(in1,HIGH);
  digitalWrite(in2,LOW);
  lcd.setCursor(0,1);             //Set cursor to second line
  lcd.print("4 high, 5 low");
  delay(3000);
  
  lcd.clear();
  lcd.home();
  lcd.print("M2 reverse");
  digitalWrite(in3,LOW);
  digitalWrite(in4,HIGH);  
  lcd.setCursor(0,1);             //Set cursor to second line
  lcd.print("6 Low, 7 High");
  delay(3000);
  digitalWrite(in4,LOW);
}
