/*
Project: GameBoy
Author: Michał Błotniak
Date: 2021
Platform: Arduino Nano
Description: Retro game written in C ++   ;)
*/

#include "Arduino.h"
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Button definitions
#define ButtonUP 2  // Yellow
#define ButtonDW 3  // Green
#define ButtonSel 4 // Red
#define ButtonRS 5  // Blue

// Marks
volatile int S1 = 0;
volatile int S2 = 0;
volatile int S3 = 0;
volatile int S4 = 0;

// Button Flags
int pushButtonUP = false;
int pushButtonDW = false;

// Interrupt functions
void goUP()
{
  pushButtonUP = true;
}

void goDW()
{
  pushButtonDW = true;
}

// Brain Game RunBOBRun
void GAME_RunBOBRun()
{
  // Variable:
  volatile int coordinate_y = 1;

  // Main Code Game:
  if (pushButtonUP == true)
  {

    switch (coordinate_y)
    {
    case 3:
      coordinate_y = 3;
      break;
    case 2:
      coordinate_y = 3;
      break;
    case 1:
      coordinate_y = 2;
      break;
    }
  }

  if (pushButtonDW == true)
  {
    switch (coordinate_y)
    {
    case 3:
      coordinate_y = 2;
      break;
    case 2:
      coordinate_y = 1;
      break;
    case 1:
      coordinate_y = 1;
      break;
    }
  }

  // Reset system
  if (digitalRead(ButtonRS) == LOW)
  {
    S1 = 1;
    S2 = 0;
    S3 = 0;
    S4 = 0;
  }
}

// Brain Game...
void GAME_()
{

  // Reset system
  if (digitalRead(ButtonRS) == LOW)
  {
    S1 = 1;
    S2 = 0;
    S3 = 0;
    S4 = 0;
  }
}

// Set up project
void setup()
{
  // LCD set
  lcd.init();
  lcd.backlight();
  pinMode(ButtonDW, INPUT_PULLUP);
  pinMode(ButtonUP, INPUT_PULLUP);
  pinMode(ButtonSel, INPUT_PULLUP);
  pinMode(ButtonRS, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ButtonUP), goUP, FALLING);
  attachInterrupt(digitalPinToInterrupt(ButtonDW), goDW, FALLING);

  lcd.setCursor(6, 0);
  lcd.print("Project:");
  lcd.setCursor(2, 1);
  lcd.print("G A M E   B O Y");
  lcd.setCursor(7, 2);
  lcd.print("Author:");
  lcd.setCursor(2, 3);
  lcd.print("Michal Blotniak");
  delay(4000);
  lcd.clear();
  S1 = 1;
}

// Main loop
void loop()
{
  while (S1 == 1)
  {
    lcd.setCursor(4, 0);
    lcd.print("Select game:");
    lcd.setCursor(2, 1);
    lcd.print("RBR  -> YellowBT");
    lcd.setCursor(2, 2);
    lcd.print("GAME2 -> GreenBT");
    lcd.setCursor(0, 3);
    lcd.print("Home->Bl");
    lcd.setCursor(10, 3);
    lcd.print("Info->Rd");

    if (digitalRead(ButtonUP) == LOW)
    {
      lcd.clear();
      S1 = 0;
      S2 = 0;
      S3 = 1;
      S4 = 0;
    }
    if (digitalRead(ButtonDW) == LOW)
    {
      lcd.clear();
      S1 = 0;
      S2 = 0;
      S3 = 0;
      S4 = 1;
    }
    if (digitalRead(ButtonSel) == LOW)
    {
      lcd.clear();
      S1 = 0;
      S2 = 1;
      S3 = 0;
      S4 = 0;
    }
  }

  while (S2 == 1)
  {
    lcd.setCursor(1, 0);
    lcd.print("Check my GitHub :)");
    lcd.setCursor(4, 1);
    lcd.print("Name: mechasB");
    lcd.setCursor(4, 2);
    lcd.print("Repositories:");
    lcd.setCursor(3, 3);
    lcd.print("G a m e  B o y");

    // Reset system
    if (digitalRead(ButtonRS) == LOW)
    {
      lcd.clear();
      S1 = 1;
      S2 = 0;
      S3 = 0;
      S4 = 0;
    }
  }

  while (S3 == 1)
  {
    lcd.setCursor(0, 0);
    lcd.print("Game1");
    // Reset system
    if (digitalRead(ButtonRS) == LOW)
    {
      lcd.clear();
      S1 = 1;
      S2 = 0;
      S3 = 0;
      S4 = 0;
    }

    // GAME_RunBOBRun();
  }

  while (S4 == 1)
  {
    lcd.setCursor(0, 0);
    lcd.print("Game2");

    // Reset system
    if (digitalRead(ButtonRS) == LOW)
    {
      lcd.clear();
      S1 = 1;
      S2 = 0;
      S3 = 0;
      S4 = 0;
    }
    // GAME_();
  }
}
