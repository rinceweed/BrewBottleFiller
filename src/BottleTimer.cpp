#include <Arduino.h>
#include "BottleTimer.h"

//#define TIMER_RELOAD        (0xF424)
#define TIMER_RELOAD        (0x9c4)

#define BOTTLE_TIME_MS      (10)

typedef struct BottleTiming
{
  bool configured;
  unsigned long configuredRate;
  unsigned long currentcount;
  unsigned long configuredCount;
} BottleTimeCounters;

static BottleTimeCounters TimerCountTers[MAX_BOTTLE_TIMERS];

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void TimerInitialise()
{
  for (int i = 0; i < MAX_BOTTLE_TIMERS; i++)
  {
    TimerCountTers[i].configured = false;
  }

  /* Timer1 consists of two major registers TCCR1A and TCCR1B which control the timers where
      TCCR1A is responsible for PWM and
      TCCR1B is used to set the prescalar value.
      Set all the bits in the TCCR1A register to 0 as we will not be using it. */
  
  cli();  //stop interrupts for till we make the settings
  /*1. First we reset the control register to amke sure we start with everything disabled.*/
  TCCR1A = 0; // Reset entire TCCR1A to 0
  TCCR1B = 0; // Reset entire TCCR1B to 0
  /*2. We set the prescalar to the desired value by changing the CS10 CS12 and CS12 bits. */
  //Set CS10 and CS11 1 so we get prescalar 64
  //Set CS12 to 1 so we get prescalar 256
  //Set OCIE1A/WGM12 to 1 so we enable compare match A
  TCCR1B = (1<<WGM12) | (1<<CS11) | (1<<CS10);
  /*3. We enable compare match mode on register A*/
  TIMSK1 = (1<<OCIE1A); 
  /*4. Compare register A to this value to get 10ms*/
  OCR1A = TIMER_RELOAD;
  sei();
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void ConfigureTimer(enum BottleTimers bt, float seconds)
{
  cli();
  TimerCountTers[bt].configuredRate = (seconds * 1000) / BOTTLE_TIME_MS;
  TimerCountTers[bt].configuredCount = 0;
  TimerCountTers[bt].currentcount = 0;
  TimerCountTers[bt].configured = true;
  sei();
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void StartCount(enum BottleTimers bt)
{
  cli();
  TimerCountTers[bt].configuredCount = 0;
  TimerCountTers[bt].currentcount = 0;
  sei();
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
unsigned long WhatIsCount(enum BottleTimers bt)
{
  return TimerCountTers[bt].currentcount;
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
ISR(TIMER1_COMPA_vect)
{
  for (int i = 0; i < MAX_BOTTLE_TIMERS; i++)
  {
    if (TimerCountTers[i].configured)
    {
      TimerCountTers[i].configuredCount++;
      if (TimerCountTers[i].configuredCount > TimerCountTers[i].configuredRate)
      {
        TimerCountTers[i].configuredCount = 0;
        TimerCountTers[i].currentcount++;
      }
    }
  }
}
