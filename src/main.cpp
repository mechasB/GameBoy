/*
Project: GameBoy
Author: Michał Błotniak
Date: 2021
Platform: Arduino Nano
Description: Retro game written in C ++   ;)

System connection:

   LCD DISPLAY:
    VCC  ->  5V
    GND  ->  GND
    SCL  ->  A5
    SDA  ->  A4

   BUTTONS (Input pull-up)
    Button_1 -> D2
    Button_2 -> D3
    Button_3 -> D4
    Button_4 -> D5 or RS PIN

*/

#include "Arduino.h"
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Button definitions
#define ButtonYellow 2
#define ButtonGreen 4
#define ButtonRed 3
#define ButtonBlue 5

/*--------- Marks---------*/
// lcd claer mark
int mark_clear_lcd = 0;

// start quizz mark
volatile int S1_Quizz_Start;

// main mark
volatile uint8_t S1 = 0;
volatile uint8_t S2 = 0;
volatile uint8_t S3 = 0;
volatile uint8_t S4 = 0;

// Button Flags
volatile bool pushButtonRed = false;
volatile bool pushButtonYellow = false;

/*---------- First game setup----------*/

#define SPRITE_RUN1 1
#define SPRITE_RUN2 2
#define SPRITE_JUMP 3
#define SPRITE_JUMP_UPPER '.' // Use the '.' character for the head
#define SPRITE_JUMP_LOWER 4
#define SPRITE_TERRAIN_EMPTY ' ' // User the ' ' character
#define SPRITE_TERRAIN_SOLID 5
#define SPRITE_TERRAIN_SOLID_RIGHT 6
#define SPRITE_TERRAIN_SOLID_LEFT 7
int Speed = 150;
int Tick = 0;
int Level = 0;
int Stage = 0;
int HighScore = 0;
#define HERO_HORIZONTAL_POSITION 1 // Horizontal position of hero on screen

#define TERRAIN_WIDTH 20
#define TERRAIN_EMPTY 0
#define TERRAIN_LOWER_BLOCK 1
#define TERRAIN_UPPER_BLOCK 2

#define HERO_POSITION_OFF 0         // Hero is invisible
#define HERO_POSITION_RUN_LOWER_1 1 // Hero is running on lower row (pose 1)
#define HERO_POSITION_RUN_LOWER_2 2 //                              (pose 2)

#define HERO_POSITION_JUMP_1 3       // Starting a jump
#define HERO_POSITION_JUMP_2 4       // Half-way up
#define HERO_POSITION_JUMP_3 5       // Jump is on upper row
#define HERO_POSITION_JUMP_4 6       // Jump is on upper row
#define HERO_POSITION_JUMP_5 7       // Jump is on upper row
#define HERO_POSITION_JUMP_6 8       // Jump is on upper row
#define HERO_POSITION_JUMP_7 9       // Half-way down
#define HERO_POSITION_JUMP_8 10      // About to land
#define HERO_POSITION_RUN_UPPER_1 11 // Hero is running on upper row (pose 1)
#define HERO_POSITION_RUN_UPPER_2 12 //                              (pose 2)

static char terrainUpper[TERRAIN_WIDTH + 1];
static char terrainLower[TERRAIN_WIDTH + 1];

void initializeGraphics()
{
  static byte graphics[] = {
      // Run position 1
      B01100,
      B01100,
      B00000,
      B01110,
      B11100,
      B01100,
      B11010,
      B10011,
      // Run position 2
      B01100,
      B01100,
      B00000,
      B01100,
      B01100,
      B01100,
      B01100,
      B01110,
      // Jump
      B01100,
      B01100,
      B00000,
      B11110,
      B01101,
      B11111,
      B10000,
      B00000,
      // Jump lower
      B11110,
      B01101,
      B11111,
      B10000,
      B00000,
      B00000,
      B00000,
      B00000,
      // Ground
      B11111,
      B11111,
      B11111,
      B11111,
      B11111,
      B11111,
      B11111,
      B11111,
      // Ground right
      B00011,
      B00011,
      B00011,
      B00011,
      B00011,
      B00011,
      B00011,
      B00011,
      // Ground left
      B11000,
      B11000,
      B11000,
      B11000,
      B11000,
      B11000,
      B11000,
      B11000,
  };
  int i;
  for (i = 0; i < 7; ++i)
  {
    lcd.createChar(i + 1, &graphics[i * 8]);
  }
  for (i = 0; i < TERRAIN_WIDTH; ++i)
  {
    terrainUpper[i] = SPRITE_TERRAIN_EMPTY;
    terrainLower[i] = SPRITE_TERRAIN_EMPTY;
  }
}
void advanceTerrain(char *terrain, byte newTerrain)
{
  for (int i = 0; i < TERRAIN_WIDTH; ++i)
  {
    char current = terrain[i];
    char next = (i == TERRAIN_WIDTH - 1) ? newTerrain : terrain[i + 1];
    switch (current)
    {
    case SPRITE_TERRAIN_EMPTY:
      terrain[i] = (next == SPRITE_TERRAIN_SOLID) ? SPRITE_TERRAIN_SOLID_RIGHT : SPRITE_TERRAIN_EMPTY;
      break;
    case SPRITE_TERRAIN_SOLID:
      terrain[i] = (next == SPRITE_TERRAIN_EMPTY) ? SPRITE_TERRAIN_SOLID_LEFT : SPRITE_TERRAIN_SOLID;
      break;
    case SPRITE_TERRAIN_SOLID_RIGHT:
      terrain[i] = SPRITE_TERRAIN_SOLID;
      break;
    case SPRITE_TERRAIN_SOLID_LEFT:
      terrain[i] = SPRITE_TERRAIN_EMPTY;
      break;
    }
  }
}

bool drawHero(byte position, char *terrainUpper, char *terrainLower, unsigned int score)
{
  bool collide = false;
  char upperSave = terrainUpper[HERO_HORIZONTAL_POSITION];
  char lowerSave = terrainLower[HERO_HORIZONTAL_POSITION];
  byte upper, lower;
  switch (position)
  {
  case HERO_POSITION_OFF:
    upper = lower = SPRITE_TERRAIN_EMPTY;
    break;
  case HERO_POSITION_RUN_LOWER_1:
    upper = SPRITE_TERRAIN_EMPTY;
    lower = SPRITE_RUN1;
    break;
  case HERO_POSITION_RUN_LOWER_2:
    upper = SPRITE_TERRAIN_EMPTY;
    lower = SPRITE_RUN2;
    break;
  case HERO_POSITION_JUMP_1:
  case HERO_POSITION_JUMP_8:
    upper = SPRITE_TERRAIN_EMPTY;
    lower = SPRITE_JUMP;
    break;
  case HERO_POSITION_JUMP_2:
  case HERO_POSITION_JUMP_7:
    upper = SPRITE_JUMP_UPPER;
    lower = SPRITE_JUMP_LOWER;
    break;
  case HERO_POSITION_JUMP_3:
  case HERO_POSITION_JUMP_4:
  case HERO_POSITION_JUMP_5:
  case HERO_POSITION_JUMP_6:
    upper = SPRITE_JUMP;
    lower = SPRITE_TERRAIN_EMPTY;
    break;
  case HERO_POSITION_RUN_UPPER_1:
    upper = SPRITE_RUN1;
    lower = SPRITE_TERRAIN_EMPTY;
    break;
  case HERO_POSITION_RUN_UPPER_2:
    upper = SPRITE_RUN2;
    lower = SPRITE_TERRAIN_EMPTY;
    break;
  }
  if (upper != ' ')
  {
    terrainUpper[HERO_HORIZONTAL_POSITION] = upper;
    collide = (upperSave == SPRITE_TERRAIN_EMPTY) ? false : true;
  }
  if (lower != ' ')
  {
    terrainLower[HERO_HORIZONTAL_POSITION] = lower;
    collide |= (lowerSave == SPRITE_TERRAIN_EMPTY) ? false : true;
  }
  byte digits = (score > 9999) ? 5 : (score > 999) ? 4
                                 : (score > 99)    ? 3
                                 : (score > 9)     ? 2
                                                   : 1;

  // Draw the scene
  terrainUpper[TERRAIN_WIDTH] = '\0';
  terrainLower[TERRAIN_WIDTH] = '\0';
  char temp = terrainUpper[16 - digits];
  terrainUpper[16 - digits] = '\0';
  lcd.setCursor(0, 0);
  lcd.print(terrainUpper);
  terrainUpper[16 - digits] = temp;
  lcd.setCursor(0, 1);
  lcd.print(terrainLower);
  lcd.setCursor(0, 3);
  lcd.print("Dist ");
  lcd.setCursor(6, 3);
  lcd.print(score);
  if (Stage == 25)
  {
    Stage = 0;
    Level++;
    Speed--;
  }
  lcd.setCursor(0, 2);
  lcd.print("Score");
  lcd.setCursor(6, 2);
  lcd.print(Level);
  terrainUpper[HERO_HORIZONTAL_POSITION] = upperSave;
  terrainLower[HERO_HORIZONTAL_POSITION] = lowerSave;
  return collide;
}

/*---------- End first game setup----------*/

// Interrupt functions
void ButtonYellowPush()
{
  pushButtonYellow = true;
}
void ButtonRedPush()
{
  pushButtonRed = true;
}

// Set up project
void setup()
{
  // lcd display setup
  lcd.init();
  lcd.backlight();

  // button set up
  pinMode(ButtonYellow, INPUT);
  pinMode(ButtonRed, OUTPUT);
  pinMode(ButtonGreen, INPUT_PULLUP);
  pinMode(ButtonBlue, INPUT_PULLUP);

  digitalWrite(ButtonRed, HIGH);
  digitalWrite(ButtonYellow, HIGH);

  // Interrupts setup
  attachInterrupt(0, ButtonYellowPush, FALLING);
  attachInterrupt(ButtonRed, ButtonRedPush, FALLING);

  // Info display
  lcd.setCursor(6, 0);
  lcd.print("Project:");
  lcd.setCursor(2, 1);
  lcd.print("G A M E   B O Y");
  lcd.setCursor(7, 2);
  lcd.print("Author:");
  lcd.setCursor(2, 3);
  lcd.print("Michal Blotniak");
  delay(500);
  lcd.clear();
  S1 = 1;
}

// Main loop
void loop()
{

  while (S1 == 1)
  {
    if (mark_clear_lcd == 1)
    {
      lcd.clear();
      mark_clear_lcd = 0;
    }

    lcd.setCursor(4, 0);
    lcd.print("Select game:");
    lcd.setCursor(2, 1);
    lcd.print("RBR  -> YellowBT");
    lcd.setCursor(1, 2);
    lcd.print("Quizz -> GreenBT");
    lcd.setCursor(1, 3);
    lcd.print("Home->Bl");
    lcd.setCursor(11, 3);
    lcd.print("Info->Rd");

    if (digitalRead(ButtonYellow) == LOW)
    {
      lcd.clear();
      S1 = 0;
      S2 = 0;
      S3 = 1;
      S4 = 0;
    }
    if (digitalRead(ButtonGreen) == LOW)
    {
      lcd.clear();
      S1 = 0;
      S2 = 0;
      S3 = 0;
      S4 = 1;
      S1_Quizz_Start = 1;
    }
    if (digitalRead(ButtonRed) == LOW)
    {
      lcd.clear();
      S1 = 0;
      S2 = 1;
      S3 = 0;
      S4 = 0;
    }
  }

  /*------------ Info display ------------*/
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
    if (digitalRead(ButtonBlue) == LOW)
    {
      lcd.clear();
      mark_clear_lcd = 1;
      S1 = 1;
      S2 = 0;
      S3 = 0;
      S4 = 0;
    }
  }

  /*--------------- Game "RBR" -------------*/
  while (S3 == 1)
  {
    static unsigned int distance = 0;
    // Reset system
    if (digitalRead(ButtonBlue) == LOW)
    {
      lcd.clear();
      mark_clear_lcd = 1;
      S1 = 1;
      S2 = 0;
      S3 = 0;
      S4 = 0;

      distance = 0;
      Level = 0;
    }
    static byte heroPos = HERO_POSITION_RUN_LOWER_1;
    static byte newTerrainType = TERRAIN_EMPTY;
    static byte newTerrainDuration = 1;
    static bool playing = false;
    static bool blink = false;

    if (!playing)
    {
      drawHero((blink) ? HERO_POSITION_OFF : heroPos, terrainUpper, terrainLower, distance >> 3);
      if (blink)
      {
        lcd.setCursor(3, 0);
        lcd.print("Press To Start ");
        delay(350);
        lcd.setCursor(3, 0);
        lcd.print("               ");
        lcd.setCursor(5, 2);
        lcd.print("    ");
        lcd.setCursor(5, 3);
        lcd.print("    ");
        Tick++;
        lcd.setCursor(11, 2);
        lcd.print("Top Score");
        lcd.setCursor(15, 3);
        lcd.print(HighScore);
        if (Tick == 50)
        {
          lcd.noBacklight();
          Tick = 0;
        }
      }
      delay(150);
      blink = !blink;
      if (pushButtonYellow)
      {
        initializeGraphics();
        heroPos = HERO_POSITION_RUN_LOWER_1;
        playing = true;
        pushButtonYellow = false;
        distance = 0;
        Level = 0;
        lcd.backlight();
      }
      return;
    }

    // Shift the terrain to the left
    advanceTerrain(terrainLower, newTerrainType == TERRAIN_LOWER_BLOCK ? SPRITE_TERRAIN_SOLID : SPRITE_TERRAIN_EMPTY);
    advanceTerrain(terrainUpper, newTerrainType == TERRAIN_UPPER_BLOCK ? SPRITE_TERRAIN_SOLID : SPRITE_TERRAIN_EMPTY);

    // Make new terrain to enter on the right
    if (--newTerrainDuration == 0)
    {
      if (newTerrainType == TERRAIN_EMPTY)
      {
        newTerrainType = (random(3) == 0) ? TERRAIN_UPPER_BLOCK : TERRAIN_LOWER_BLOCK;
        newTerrainDuration = 2 + random(10);
      }
      else
      {
        newTerrainType = TERRAIN_EMPTY;
        newTerrainDuration = 10 + random(10);
      }
    }

    if (pushButtonYellow)
    {
      if (heroPos <= HERO_POSITION_RUN_LOWER_2)
        heroPos = HERO_POSITION_JUMP_1;
      pushButtonYellow = false;
    }

    if (drawHero(heroPos, terrainUpper, terrainLower, distance >> 3))
    {
      playing = false; // The hero collided with something. Too bad.
    }
    else
    {
      if (heroPos == HERO_POSITION_RUN_LOWER_2 || heroPos == HERO_POSITION_JUMP_8)
      {
        heroPos = HERO_POSITION_RUN_LOWER_1;
      }
      else if ((heroPos >= HERO_POSITION_JUMP_3 && heroPos <= HERO_POSITION_JUMP_5) && terrainLower[HERO_HORIZONTAL_POSITION] != SPRITE_TERRAIN_EMPTY)
      {
        heroPos = HERO_POSITION_RUN_UPPER_1;
      }
      else if (heroPos >= HERO_POSITION_RUN_UPPER_1 && terrainLower[HERO_HORIZONTAL_POSITION] == SPRITE_TERRAIN_EMPTY)
      {
        heroPos = HERO_POSITION_JUMP_5;
      }
      else if (heroPos == HERO_POSITION_RUN_UPPER_2)
      {
        heroPos = HERO_POSITION_RUN_UPPER_1;
      }
      else
      {
        ++heroPos;
      }
      ++distance;
      Stage++;
      if (Level > HighScore)
      {
        HighScore = Level;
      }
      lcd.setCursor(11, 2);
      lcd.print("Top Score");
      lcd.setCursor(15, 3);
      lcd.print(HighScore);
      digitalWrite(ButtonRed, terrainLower[HERO_HORIZONTAL_POSITION + 2] == SPRITE_TERRAIN_EMPTY ? HIGH : LOW);
    }
    delay(Speed);
  }

  /*--------------- End Game "RBR" -------------*/

  /*--------------- Game "Quizz" -------------*/

  while (S4 == 1)
  {

    /*--------------------Set Quizz--------------------*/
    // Marks

    uint8_t S_End_Quizz = 0;   // lose mark
    uint8_t S_Finish_Game = 0; // end game mark
    uint8_t S1_Quizz = 0;
    uint8_t S2_Quizz = 0;
    uint8_t S3_Quizz = 0;
    uint8_t S4_Quizz = 0;
    uint8_t S5_Quizz = 0;
    uint8_t S6_Quizz = 0;
    uint8_t S7_Quizz = 0;
    uint8_t S8_Quizz = 0;
    uint8_t S9_Quizz = 0;
    uint8_t S10_Quizz = 0;

    /*-------------Reset system--------------*/
    if (digitalRead(ButtonBlue) == LOW)
    {
      lcd.clear();
      mark_clear_lcd = 1;
      S1 = 1;
      S2 = 0;
      S3 = 0;
      S4 = 0;
      S1_Quizz_Start = 0;
      S_End_Quizz = 0;
      S1_Quizz = 0;
      S2_Quizz = 0;
      S3_Quizz = 0;
      S4_Quizz = 0;
      S5_Quizz = 0;
      S6_Quizz = 0;
      S7_Quizz = 0;
      S8_Quizz = 0;
      S9_Quizz = 0;
      S10_Quizz = 0;
    }

    /*--------End display----------*/
    while (S_End_Quizz == 1)
    {
      lcd.setCursor(4, 0);
      lcd.print("?  Quizz  ?");
      lcd.setCursor(3, 1);
      lcd.print("Bad answer :/");
      lcd.setCursor(0, 2);
      lcd.print("Play again  Go home");
      lcd.setCursor(0, 3);
      lcd.print("  <---       --->  ");

      if (pushButtonYellow)
      {
        S1_Quizz_Start = 1;
        S_End_Quizz = 0;
        S1_Quizz = 0;
        S2_Quizz = 0;
        S3_Quizz = 0;
        S4_Quizz = 0;
        S5_Quizz = 0;
        S6_Quizz = 0;
        S7_Quizz = 0;
        S8_Quizz = 0;
        S9_Quizz = 0;
        S10_Quizz = 0;
      }
      if (digitalRead(ButtonBlue) == LOW)
      {
        lcd.clear();
        mark_clear_lcd = 1;
        S1 = 1;
        S2 = 0;
        S3 = 0;
        S4 = 0;
      }
    }

    /*---------- Finish display----------*/
    while (S_Finish_Game == 1)
    {
      lcd.setCursor(1, 1);
      lcd.print("Congratulations !");
      lcd.setCursor(2, 2);
      lcd.print("You know a lot ");
      lcd.setCursor(2, 3);
      lcd.print("about arduino ;)");
      delay(5000);
      lcd.clear();
      S_Finish_Game = 0;
      S1 = 1;
    }

    /*--------------------Brain Quizz--------------------*/

    while (S1_Quizz_Start == 1)
    {
      lcd.setCursor(4, 0);
      lcd.print("?  Quizz  ?");
      lcd.setCursor(0, 1);
      lcd.print(" Select the correct ");
      lcd.setCursor(0, 2);
      lcd.print("answer using the bt");
      lcd.setCursor(0, 3);
      lcd.print("<- Lf_ans   Rg_ans->");
      delay(3000);
      lcd.clear();
      S1_Quizz_Start = 0;
      S1_Quizz = 1;
    }

    // first question
    while (S1_Quizz == 1)
    {
      lcd.setCursor(3, 0);
      lcd.print("What is blink:");
      lcd.setCursor(0, 2);
      lcd.print("<--  blinking LED");
      lcd.setCursor(2, 3);
      lcd.print("IDE for Arduino-->");
      if (pushButtonRed)
      {
        lcd.clear();
        S2_Quizz = 1;
        S1_Quizz = 0;
      }
      if (pushButtonYellow)
      {
        lcd.clear();
        S_End_Quizz = 1;
        S1_Quizz = 0;
      }
    }

    // second question
    while (S2_Quizz == 1)
    {
      lcd.setCursor(0, 0);
      lcd.print(" Where the Arduino ");
      lcd.setCursor(2, 1);
      lcd.print("was constructed ?");
      lcd.setCursor(3, 3);
      lcd.print("<- In Italy");
      lcd.setCursor(5, 3);
      lcd.print("In USA ->");
      if (pushButtonRed)
      {
        lcd.clear();
        S2_Quizz = 0;
        S3_Quizz = 1;
      }
      if (pushButtonYellow)
      {
        lcd.clear();
        S2_Quizz = 0;
        S_End_Quizz = 1;
      }
    }

    // third question
    while (S3_Quizz == 1)
    {
      lcd.setCursor(0, 0);
      lcd.print("What language do we");
      lcd.setCursor(1, 1);
      lcd.print("program arduino in?");
      lcd.setCursor(0, 2);
      lcd.print("<- HTML");
      lcd.setCursor(13, 3);
      lcd.print("C++ ->");
      if (pushButtonRed)
      {
        lcd.clear();
        S3_Quizz = 0;
        S_End_Quizz = 1;
      }
      if (pushButtonYellow)
      {
        lcd.clear();
        S3_Quizz = 0;
        S4_Quizz = 1;
      }
    }

    // fourth question
    while (S4_Quizz == 1)
    {
      lcd.setCursor(0, 0);
      lcd.print(" When was the C ? ");
      lcd.setCursor(3, 3);
      lcd.print("<- 1972");
      lcd.setCursor(5, 3);
      lcd.print("2000 ->");
      if (pushButtonRed)
      {
        lcd.clear();
        S4_Quizz = 0;
        S5_Quizz = 1;
      }
      if (pushButtonYellow)
      {
        lcd.clear();
        S4_Quizz = 0;
        S_End_Quizz = 1;
      }
    }

    // fifth question
    while (S5_Quizz == 1)
    {
      lcd.setCursor(0, 0);
      lcd.print("What is && in C++ ?");
      lcd.setCursor(0, 2);
      lcd.print("<- Product (AND)");
      lcd.setCursor(6, 3);
      lcd.print("Sum (OR) ->");
      if (pushButtonRed)
      {
        lcd.clear();
        S5_Quizz = 0;
        S_End_Quizz = 1;
      }
      if (pushButtonYellow)
      {
        lcd.clear();
        S5_Quizz = 0;
        S6_Quizz = 1;
      }
    }

    // sixth question
    while (S6_Quizz == 1)
    {
      lcd.setCursor(0, 0);
      lcd.print("What processors are");
      lcd.setCursor(4, 1);
      lcd.print("in Arduino ?");
      lcd.setCursor(0, 2);
      lcd.print("<-- STM8");
      lcd.setCursor(7, 3);
      lcd.print("Atmel AVR-->");
      if (pushButtonYellow)
      {
        lcd.clear();
        S6_Quizz = 1;
        S7_Quizz = 0;
      }
      if (pushButtonRed)
      {
        lcd.clear();
        S_End_Quizz = 1;
        S7_Quizz = 0;
      }
    }

    // seventh question
    while (S7_Quizz == 1)
    {
      lcd.setCursor(0, 0);
      lcd.print(" Who is the author ");
      lcd.setCursor(4, 1);
      lcd.print("of arduino ?");
      lcd.setCursor(0, 2);
      lcd.print("<- Massimo Banzi");
      lcd.setCursor(7, 3);
      lcd.print("Bill Gates ->");
      if (pushButtonRed)
      {
        lcd.clear();
        S7_Quizz = 0;
        S8_Quizz = 1;
      }
      if (pushButtonYellow)
      {
        lcd.clear();
        S8_Quizz = 0;
        S_End_Quizz = 1;
      }
    }

    // eighth question
    while (S8_Quizz == 1)
    {
      lcd.setCursor(5, 0);
      lcd.print("What year was");
      lcd.setCursor(1, 1);
      lcd.print("the arduino made?");
      lcd.setCursor(0, 2);
      lcd.print("<- 2005");
      lcd.setCursor(12, 3);
      lcd.print("1999 ->");
      if (pushButtonYellow)
      {
        lcd.clear();
        S8_Quizz = 0;
        S_End_Quizz = 1;
      }
      if (pushButtonRed)
      {
        lcd.clear();
        S8_Quizz = 0;
        S9_Quizz = 1;
      }
    }

    // ninth question
    while (S9_Quizz == 1)
    {
      lcd.setCursor(1, 0);
      lcd.print("For whom arduino");
      lcd.setCursor(5, 1);
      lcd.print("was made ?");
      lcd.setCursor(0, 3);
      lcd.print("<- For developers");
      lcd.setCursor(4, 3);
      lcd.print("For students ->");
      if (pushButtonYellow)
      {
        lcd.clear();
        S9_Quizz = 0;
        S10_Quizz = 1;
      }
      if (pushButtonRed)
      {
        lcd.clear();
        S4_Quizz = 0;
        S_End_Quizz = 1;
      }
    }

    // tenth question
    while (S10_Quizz == 1)
    {
      lcd.setCursor(1, 0);
      lcd.print("How many versions");
      lcd.setCursor(1, 1);
      lcd.print("of ard are there?");
      lcd.setCursor(0, 2);
      lcd.print("<- 34");
      lcd.setCursor(14, 3);
      lcd.print("12 ->");
      if (pushButtonYellow)
      {
        lcd.clear();
        S10_Quizz = 0;
        S_End_Quizz = 1;
      }
      if (pushButtonRed)
      {
        lcd.clear();
        S10_Quizz = 0;
        S_Finish_Game = 1;
      }
    }

    /*--------------------End Brain Quizz--------------------*/
  }
}