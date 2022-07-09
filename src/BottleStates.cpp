/*--[ Include Files ]------------------------------------------------------------------------------------------------------------*/
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "BottleStates.h"
#include "BottleTimer.h"

/*--[ Types ]--------------------------------------------------------------------------------------------------------------------*/
typedef enum FillerStateList
{
  fillerIdle,
  fillerFLow,
  fillBottle,
  modeFillBottle,
  modeFillProgram,
  programBottle,
  debounce,
  maxFillerStates
} FillerStates;

typedef struct PinDebouncing
{
  uint8_t button;
  bool    buttonState;
  bool    currentButtonState;
  unsigned long debounceTime;
} PinDebounce;

typedef struct StateDebounceNavigate
{
  FillerStates goOn;
  FillerStates comeBack;
  PinDebounce  *pressedButton;
} DebounceNavigate;

typedef void (*StateFunctionHandler)();

/*--[ Prototypes ]---------------------------------------------------------------------------------------------------------------*/
void CheckButtonPress(PinDebounce *fPinIn);

void FillerIdle();
void FillerFLow();
void FillBottle();
void ModeFillBottle();
void ModeFillProgram();
void ProgramBottle();
void Debounce();

/*--[ Data ]---------------------------------------------------------------------------------------------------------------------*/
float aCurrentProgress;
unsigned long aBottleFillingTime;
unsigned long aBottleFillingComplete;
DebounceNavigate aDebounce;
FillerStates aCurrentState;
FillerStates aPreviousState;
PinDebounce aButtonDebounce[MAX_BOTTLE_BUTTONS] =
{
  { button : BOTTLE_IN_PIN_STOP, buttonState : false,  currentButtonState : false, debounceTime : 20},
  { button : BOTTLE_IN_PIN_MODE, buttonState : false,  currentButtonState : false, debounceTime : 20},
  { button : BOTTLE_IN_PIN_FILL, buttonState : false,  currentButtonState : false, debounceTime : 20},
};
StateFunctionHandler aBottleState[maxFillerStates] = 
  {
    FillerIdle,
    FillerFLow,
    FillBottle,
    ModeFillBottle,
    ModeFillProgram,
    ProgramBottle,
    Debounce
  };

static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void FillerIdle()
{
  if (aCurrentState != aPreviousState)
  {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Idle"));
    display.display();
  }

  if (aButtonDebounce[BOTTLE_IN_MODE].buttonState == true)
  {
    aDebounce.goOn = modeFillBottle;
    aDebounce.pressedButton = &(aButtonDebounce[BOTTLE_IN_MODE]);
    aCurrentState = debounce;
  }

  if (aButtonDebounce[BOTTLE_IN_FILL].buttonState == true)
  {
    aDebounce.goOn = fillerFLow;
    aDebounce.pressedButton = &(aButtonDebounce[BOTTLE_IN_FILL]);
    aCurrentState = debounce;
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void FillerFLow()
{
  if (aCurrentState != aPreviousState)
  {
    digitalWrite(BOTTLE_OUT_PIN_PWM, BOTTLE_FILL_SPEED);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Free-FLow"));
    display.display();
    aPreviousState = aCurrentState;
  }

  if (aButtonDebounce[BOTTLE_IN_STOP].buttonState == true)
  {
    digitalWrite(BOTTLE_OUT_PIN_PWM, BOTTLE_STOP_SPEED);
    aDebounce.goOn = fillerIdle;
    aDebounce.pressedButton = &(aButtonDebounce[BOTTLE_IN_STOP]);
    aCurrentState = debounce;
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void FillBottle()
{
  if (aCurrentState != aPreviousState)
  {
    StartCount(BOTTLETIME_FILL);
    digitalWrite(BOTTLE_OUT_PIN_PWM, BOTTLE_FILL_SPEED);
    aCurrentProgress = 0;
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Filling !"));
    display.setCursor(20, 30);
    display.print(0, 0);
    display.println(" %");
    display.display();
    aPreviousState = aCurrentState;
  }

  aBottleFillingComplete = WhatIsCount(BOTTLETIME_FILL);
  float progress = (aBottleFillingComplete * 100) / aBottleFillingTime;
  if (aCurrentProgress != progress)
  {
    aCurrentProgress = progress;
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Filling !"));
    display.setCursor(20, 30);
    display.print(progress, 0);
    display.println(" %");
    display.display();
  }

  if (aBottleFillingComplete > aBottleFillingTime)
  {
    digitalWrite(BOTTLE_OUT_PIN_PWM, BOTTLE_STOP_SPEED);
    aCurrentState = modeFillBottle;
  }

  if (aButtonDebounce[BOTTLE_IN_STOP].buttonState == true)
  {
    aDebounce.goOn = fillerIdle;
    aDebounce.pressedButton = &(aButtonDebounce[BOTTLE_IN_STOP]);
    aCurrentState = debounce;
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void ModeFillBottle()
{
  if (aCurrentState != aPreviousState)
  {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Mode->Fill"));
    display.display();
    aPreviousState = aCurrentState;
  }

  if (aButtonDebounce[BOTTLE_IN_MODE].buttonState == true)
  {
    aDebounce.goOn = modeFillProgram;
    aDebounce.pressedButton = &(aButtonDebounce[BOTTLE_IN_MODE]);
    aCurrentState = debounce;
  }

  if (aButtonDebounce[BOTTLE_IN_FILL].buttonState == true)
  {
    aDebounce.goOn = fillBottle;
    aDebounce.pressedButton = &(aButtonDebounce[BOTTLE_IN_FILL]);
    aCurrentState = debounce;
  }

  if (aButtonDebounce[BOTTLE_IN_STOP].buttonState == true)
  {
    aDebounce.goOn = fillerIdle;
    aDebounce.pressedButton = &(aButtonDebounce[BOTTLE_IN_STOP]);
    aCurrentState = debounce;
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void ModeFillProgram()
{
  if (aCurrentState != aPreviousState)
  {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Mode->Prog"));
    display.display();
    aPreviousState = aCurrentState;
  }

  if (aButtonDebounce[BOTTLE_IN_MODE].buttonState == true)
  {
    aDebounce.goOn = modeFillBottle;
    aDebounce.pressedButton = &(aButtonDebounce[BOTTLE_IN_MODE]);
    aCurrentState = debounce;
  }

  if (aButtonDebounce[BOTTLE_IN_FILL].buttonState == true)
  {
    aDebounce.goOn = programBottle;
    aDebounce.pressedButton = &(aButtonDebounce[BOTTLE_IN_FILL]);
    aCurrentState = debounce;
  }

  if (aButtonDebounce[BOTTLE_IN_STOP].buttonState == true)
  {
    aDebounce.goOn = fillerIdle;
    aDebounce.pressedButton = &(aButtonDebounce[BOTTLE_IN_STOP]);
    aCurrentState = debounce;
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void ProgramBottle()
{
  if (aCurrentState != aPreviousState)
  {
    StartCount(BOTTLETIME_FILL);
    digitalWrite(BOTTLE_OUT_PIN_PWM, BOTTLE_FILL_SPEED);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Program ->"));
    display.println(F("Filling"));
    display.println(F("Bottle"));
    display.display();
    aPreviousState = aCurrentState;
  }
  
  if (aButtonDebounce[BOTTLE_IN_STOP].buttonState == true)
  {
    digitalWrite(BOTTLE_OUT_PIN_PWM, BOTTLE_STOP_SPEED);
    // Roll-Over not taken into account
    aBottleFillingTime = WhatIsCount(BOTTLETIME_FILL);
    aDebounce.goOn = modeFillProgram;
    aDebounce.pressedButton = &(aButtonDebounce[BOTTLE_IN_STOP]);
    aCurrentState = debounce;
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void Debounce()
{
  if (aDebounce.pressedButton->buttonState == false)
  {
    aCurrentState = aDebounce.goOn;
    aPreviousState = debounce;
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void BottleStatesInitialise()
{
  for (uint8_t i = 0; i< MAX_BOTTLE_BUTTONS; i++)
  {
    pinMode(aButtonDebounce[i].button, INPUT_PULLUP);
  }

  pinMode(BOTTLE_OUT_PIN_PWM, OUTPUT);
  ConfigureTimer(BOTTLETIME_FILL, 1);
  ConfigureTimer(BOTTLETIME_KEYPRESS, 0.01);

  aCurrentState = fillerIdle;
  aPreviousState = debounce;

  aBottleFillingTime = 302; //301500 ms

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Clear the buffer
  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(F("  Bottle"));
  display.println(F("   Fill"));
  display.display();      // Show initial text
  delay(1000);
  display.clearDisplay();
  display.display();      // Show initial text
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void BottleStatesExecute()
{
  for (uint8_t i = 0; i < MAX_BOTTLE_BUTTONS; i++)
  {
    CheckButtonPress(&(aButtonDebounce[i]));
  }
  aBottleState[aCurrentState]();
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void CheckButtonPress(PinDebounce *fPinIn)
{
  // read the state of the switch into a local variable:
  bool value = digitalRead(fPinIn->button) ? false : true;
  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (value != fPinIn->currentButtonState)
  {
    // reset the debouncing timer
    fPinIn->debounceTime = WhatIsCount(BOTTLETIME_KEYPRESS);
    fPinIn->currentButtonState = value;
  }

  unsigned long current_time = WhatIsCount(BOTTLETIME_KEYPRESS);
  unsigned long high_time = current_time;
  unsigned long low_time = fPinIn->debounceTime;
  if (current_time < fPinIn->debounceTime)
  {
    low_time = current_time;
    high_time = fPinIn->debounceTime + (fPinIn->debounceTime - current_time);
  }

  unsigned long debounce_time = high_time - low_time;

  if (debounce_time > BOTTLE_IN_DEBOUNCE_TIME)
  {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:
    fPinIn->buttonState = value;
  }
}
