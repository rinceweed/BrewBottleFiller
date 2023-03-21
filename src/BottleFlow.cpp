/*--[ Include Files ]------------------------------------------------------------------------------------------------------------*/
#include <Arduino.h>
#include "BottleTimer.h"
#include "BottleFlow.h"

#define BOTTLE_FLOW_PIN              (2)
#define BOTTLE_FLOW_INTERRUPT        (0) // 0 for pin 2
#define BOTTLE_FLOW_PULSES_PER_LITER (5880.0)//1L = 5880 square waves
#define BOTTLE_FLOW_TMER_MS          (100.0)
#define BOTTLE_FLOW_TMER_PER_S       ((float)(BOTTLE_FLOW_TMER_MS / 1000.0))
#define BOTTLE_FLOW_RATE_PER_S       ((float)(BOTTLE_FLOW_PULSES_PER_LITER / 60 / BOTTLE_FLOW_TMER_PER_S))

/*--[ Data ]---------------------------------------------------------------------------------------------------------------------*/
double flowRate;
volatile unsigned long totalFlowCount;
volatile unsigned long flowRateCount;

/*--[ Prototypes ]---------------------------------------------------------------------------------------------------------------*/
void FlowInterrupt();

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void FlowInitialise()
{
  pinMode(BOTTLE_FLOW_PIN, INPUT);
  attachInterrupt(BOTTLE_FLOW_INTERRUPT, FlowInterrupt, RISING);
  ConfigureTimer(BOTTLETIME_FLOW, BOTTLE_FLOW_TMER_PER_S);
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void CalculateFlowRateCyclically()
{
  unsigned long current_time = WhatIsCount(BOTTLETIME_FLOW);
  // Should increment every BOTTLE_FLOW_TMER_MS
  if (current_time > 1)
  {
    flowRate = (flowRateCount / BOTTLE_FLOW_RATE_PER_S) * 60.0; // lit / minute
    StartCount(BOTTLETIME_FLOW);
    flowRateCount = 0;
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void StartFlow()
{
  StartCount(BOTTLETIME_FLOW);
  totalFlowCount = 0;
  flowRate = 0;
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
unsigned long WhatIsFlowCount()
{
  return totalFlowCount;
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
float WhatIsFlowRate()
{
  return flowRate;
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void FlowInterrupt()
{
  totalFlowCount++;
  flowRateCount++;
}
