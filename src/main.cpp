#include <Arduino.h>
#include "BottleStates.h"
#include "BottleTimer.h"

///-----------------------------------------------------------------------------------------------------
void setup()
{
  TimerInitialise();
  BottleStatesInitialise();
}

///-----------------------------------------------------------------------------------------------------
void loop()
{
  BottleStatesExecute();
}