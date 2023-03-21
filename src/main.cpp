#include <Arduino.h>
#include "BottleStates.h"
#include "BottleTimer.h"
#include "BottleFlow.h"

///-----------------------------------------------------------------------------------------------------
void setup()
{
  TimerInitialise();
  BottleStatesInitialise();
  FlowInitialise();
}

///-----------------------------------------------------------------------------------------------------
void loop()
{
  BottleStatesExecute();
  CalculateFlowRateCyclically();
}