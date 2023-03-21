/*--[ Include Files ]------------------------------------------------------------------------------------------------------------*/
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "BottleStates.h"
#include "BottleTimer.h"
#include "BottleFlow.h"

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
unsigned long aBottleFillingCount;
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

static Adafruit_SSD1306 aOledDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
unsigned long flowcount;
float flowrate;

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void FillerIdle()
{
  // Entry action
  if (aCurrentState != aPreviousState)
  {
    aOledDisplay.clearDisplay();
    aOledDisplay.setCursor(0, 0);
    aOledDisplay.println(F("Idle"));
    aOledDisplay.display();
  }

  // Continious action

  // Rules action
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

  // Exit action
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void FillerFLow()
{
  // Entry action
  if (aCurrentState != aPreviousState)
  {
    digitalWrite(BOTTLE_OUT_PIN_PWM, BOTTLE_FILL_SPEED);
    aOledDisplay.clearDisplay();
    aOledDisplay.setCursor(0, 0);
    aOledDisplay.println(F("Free-FLow"));
    aOledDisplay.display();
    aPreviousState = aCurrentState;

    flowcount = WhatIsFlowCount();
    StartFlow();
  }

  // Continious action
  unsigned long current = WhatIsFlowCount();
  if (flowcount != current)
  {
    flowcount = WhatIsFlowCount();
    aOledDisplay.clearDisplay();
    aOledDisplay.setCursor(0, 0);
    aOledDisplay.println(F("Free-FLow"));
    aOledDisplay.setCursor(20, 30);
    aOledDisplay.print(flowcount);
    aOledDisplay.print(F(" "));
    flowrate = WhatIsFlowRate();
    aOledDisplay.print(flowrate);
    aOledDisplay.display();
  }

  // Rules action
  if (aButtonDebounce[BOTTLE_IN_STOP].buttonState == true)
  {
    aDebounce.goOn = fillerIdle;
    aDebounce.pressedButton = &(aButtonDebounce[BOTTLE_IN_STOP]);
    aCurrentState = debounce;
  }

  // Exit action
  if (aCurrentState != aPreviousState)
  {
    digitalWrite(BOTTLE_OUT_PIN_PWM, BOTTLE_STOP_SPEED);
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void FillBottle()
{
  // Entry action
  if (aCurrentState != aPreviousState)
  {
    StartFlow();
    digitalWrite(BOTTLE_OUT_PIN_PWM, BOTTLE_FILL_SPEED);
    aCurrentProgress = 0;
    aOledDisplay.clearDisplay();
    aOledDisplay.setCursor(0, 0);
    aOledDisplay.println(F("Filling !"));
    aOledDisplay.setCursor(20, 30);
    aOledDisplay.print(0, 0);
    aOledDisplay.println("%");
    aOledDisplay.display();
    aPreviousState = aCurrentState;
  }

  // Continious action
  aBottleFillingComplete = WhatIsFlowCount();
  float progress = (aBottleFillingComplete * 100) / aBottleFillingCount;
  if (aCurrentProgress != progress)
  {
    aCurrentProgress = progress;
    aOledDisplay.clearDisplay();
    aOledDisplay.setCursor(0, 0);
    aOledDisplay.println(F("Filling !"));
    aOledDisplay.setCursor(20, 30);
    aOledDisplay.print(progress, 0);
    aOledDisplay.println("%");
    aOledDisplay.print(F(" "));
    flowrate = WhatIsFlowRate();
    aOledDisplay.print(flowrate);
    aOledDisplay.print(F("L/min"));
    aOledDisplay.display();
  }

  // Rules action
  if (aBottleFillingComplete > aBottleFillingCount)
  {
    aCurrentState = modeFillBottle;
  }

  if (aButtonDebounce[BOTTLE_IN_STOP].buttonState == true)
  {
    aDebounce.goOn = fillerIdle;
    aDebounce.pressedButton = &(aButtonDebounce[BOTTLE_IN_STOP]);
    aCurrentState = debounce;
  }

  // Exit action
  if (aCurrentState != aPreviousState)
  {
    digitalWrite(BOTTLE_OUT_PIN_PWM, BOTTLE_STOP_SPEED);
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void ModeFillBottle()
{
  // Entry action
  if (aCurrentState != aPreviousState)
  {
    aOledDisplay.clearDisplay();
    aOledDisplay.setCursor(0, 0);
    aOledDisplay.println(F("Mode->Fill"));
    aOledDisplay.display();
    aPreviousState = aCurrentState;
  }

  // Continious action

  // Rules action
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

  // Exit action
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void ModeFillProgram()
{
  // Entry action
  if (aCurrentState != aPreviousState)
  {
    aOledDisplay.clearDisplay();
    aOledDisplay.setCursor(0, 0);
    aOledDisplay.println(F("Mode->Prog"));
    aOledDisplay.display();
    aPreviousState = aCurrentState;
  }

  // Continious action

  // Rules action
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

  // Exit action
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void ProgramBottle()
{
  // Entry action
  if (aCurrentState != aPreviousState)
  {
    StartFlow();
    digitalWrite(BOTTLE_OUT_PIN_PWM, BOTTLE_FILL_SPEED);
    aOledDisplay.clearDisplay();
    aOledDisplay.setCursor(0, 0);
    aOledDisplay.println(F("Program ->"));
    aOledDisplay.println(F("Filling"));
    aOledDisplay.println(F("Bottle"));
    aOledDisplay.display();
    aPreviousState = aCurrentState;
  }
  
  // Continious action

  // Rules action
  if (aButtonDebounce[BOTTLE_IN_STOP].buttonState == true)
  {
    // Roll-Over not taken into account
    aBottleFillingCount = WhatIsFlowCount();
    aDebounce.goOn = modeFillProgram;
    aDebounce.pressedButton = &(aButtonDebounce[BOTTLE_IN_STOP]);
    aCurrentState = debounce;
  }

  // Exit action
  if (aCurrentState != aPreviousState)
  {
    digitalWrite(BOTTLE_OUT_PIN_PWM, BOTTLE_STOP_SPEED);
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

  aBottleFillingCount = 2528; // counts for 430ml: 1lit=5880

  if(!aOledDisplay.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed. Not starting up"));
    for(;;); // Don't proceed, loop forever
  }

  // Clear the buffer
  aOledDisplay.clearDisplay();
  aOledDisplay.setTextSize(2); // Draw 2X-scale text
  aOledDisplay.setTextColor(WHITE);
  aOledDisplay.setCursor(0, 0);
  aOledDisplay.println(F("  Bottle"));
  aOledDisplay.println(F("   Fill"));
  aOledDisplay.display();      // Show initial text
  delay(1000);
  aOledDisplay.clearDisplay();
  aOledDisplay.display();      // Show initial text
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
