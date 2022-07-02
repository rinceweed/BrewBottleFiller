/*--[ Include Files ]------------------------------------------------------------------------------------------------------------*/
#include <Arduino.h>
#include "BottleStates.h"

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
unsigned long aCurrentBottleFillingTime;
unsigned long aBottleFillingTime;
DebounceNavigate aDebounce;
FillerStates aCurrentState;
FillerStates aPreviousState;
PinDebounce aButtonDebounce[MAX_BOTTLE_BUTTONS];
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


/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void FillerIdle()
{
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
    analogWrite(BOTTLE_OUT_PWM, BOTTLE_FILL_SPEED);
    aPreviousState = aCurrentState;
  }

  if (aButtonDebounce[BOTTLE_IN_STOP].buttonState == true)
  {
    analogWrite(BOTTLE_OUT_PWM, BOTTLE_STOP_SPEED);
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
    aCurrentBottleFillingTime = millis();
    analogWrite(BOTTLE_OUT_PWM, BOTTLE_FILL_SPEED);
    aPreviousState = aCurrentState;
  }

  if (millis() - aCurrentBottleFillingTime > aBottleFillingTime)
  {
    analogWrite(BOTTLE_OUT_PWM, BOTTLE_STOP_SPEED);
    aCurrentState = modeFillBottle;
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void ModeFillBottle()
{
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
    aCurrentBottleFillingTime = millis();
    analogWrite(BOTTLE_OUT_PWM, BOTTLE_FILL_SPEED);
    aPreviousState = aCurrentState;
  }
  
  if (aButtonDebounce[BOTTLE_IN_STOP].buttonState == true)
  {
    analogWrite(BOTTLE_OUT_PWM, BOTTLE_STOP_SPEED);
    aBottleFillingTime = millis() - aCurrentBottleFillingTime;
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
  pinMode(BOTTLE_IN_STOP, INPUT_PULLUP);
  pinMode(BOTTLE_IN_MODE, INPUT_PULLUP);
  pinMode(BOTTLE_IN_FILL, INPUT_PULLUP);

  aCurrentState = fillerIdle;
  aPreviousState = debounce;

  aBottleFillingTime = 0;

  // Pins D5 and D6 are 62.5kHz
  //TCCR0B = 0b00000001; // x1
  //TCCR0A = 0b00000011; // fast pwm
  pinMode(BOTTLE_OUT_PWM, OUTPUT);
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void BottleStatesExecute()
{
  for (uint8_t i = 0; i< MAX_BOTTLE_BUTTONS; i++)
  {
    CheckButtonPress(&(aButtonDebounce[i]));
  }
  aBottleState[aCurrentState]();
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void CheckButtonPress(PinDebounce *fPinIn)
{
  // read the state of the switch into a local variable:
  bool value = digitalRead(fPinIn->button) ? true : false;

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (value != fPinIn->currentButtonState)
  {
    // reset the debouncing timer
    fPinIn->debounceTime = millis();
    fPinIn->currentButtonState = value;
  }

  if ((millis() - fPinIn->debounceTime) > BOTTLE_IN_DEBOUNCE_TIME)
  {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:
    fPinIn->buttonState = value;
  }
}
