#ifndef BOTTLETIMER_H_
#define BOTTLETIMER_H_

/*--[ Include Files ]------------------------------------------------------------------------------------------------------------*/

/*--[ Literals ]-----------------------------------------------------------------------------------------------------------------*/

/*--[ Types ]--------------------------------------------------------------------------------------------------------------------*/
enum BottleTimers
{
  BOTTLETIME_KEYPRESS = 0,
  BOTTLETIME_FILL,
  MAX_BOTTLE_TIMERS
};

/*--[ Prototypes ]---------------------------------------------------------------------------------------------------------------*/
void TimerInitialise();
void ConfigureTimer(enum BottleTimers bt, float seconds);
void StartCount(enum BottleTimers bt);
unsigned long WhatIsCount(enum BottleTimers bt);

/*EOF============================================================================================================================*/
#endif