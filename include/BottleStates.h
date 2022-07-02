#ifndef BOTTLESTATES_H_
#define BOTTLESTATES_H_
/*--[ Include Files ]------------------------------------------------------------------------------------------------------------*/

/*--[ Literals ]-----------------------------------------------------------------------------------------------------------------*/
#define  BOTTLE_OUT_PWM           PD5
#define  BOTTLE_IN_DEBOUNCE_TIME  50
#define  BOTTLE_FILL_SPEED        250
#define  BOTTLE_STOP_SPEED        0

/*--[ Types ]--------------------------------------------------------------------------------------------------------------------*/
enum BottleButtons
{
  BOTTLE_IN_STOP  =  PD6,
  BOTTLE_IN_MODE  =  PD7,
  BOTTLE_IN_FILL  =  PD4,
  MAX_BOTTLE_BUTTONS = 3
};

/*--[ Prototypes ]---------------------------------------------------------------------------------------------------------------*/
void BottleStatesInitialise();
void BottleStatesExecute();

/*EOF============================================================================================================================*/
#endif
