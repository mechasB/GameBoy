/*
Project: GameBoy
Author: Michał Błotniak
Date: 2021
Platform: Arduino Nano
Description: Retro game written in C ++   ;)
*/

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

#define ButtonUP 2
#define ButtonDW 3
#define SELECT 4

int pushButtonUP = false;
int pushButtonDW = false;

void goUP()
{
  pushButtonUP = true;
}

void goDW()
{
  pushButtonDW = true;
}

void setup()
{
  // LCD set
  lcd.init();
  lcd.backlight();
  // Input set
  pinMode(ButtonDW, INPUT_PULLUP);
  pinMode(ButtonUP, INPUT_PULLUP);
  pinMode(SELECT, INPUT_PULLUP);
  // Interruptions
  attachInterrupt(digitalPinToInterrupt(ButtonUP), goUP, FALLING);
  attachInterrupt(digitalPinToInterrupt(ButtonDW), goDW, FALLING);
}

void loop()
{
  //  main code
}