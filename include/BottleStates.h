#ifndef BOTTLESTATES_H_
#define BOTTLESTATES_H_
/*--[ Include Files ]------------------------------------------------------------------------------------------------------------*/

/*--[ Literals ]-----------------------------------------------------------------------------------------------------------------*/
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
//#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)


#define  BOTTLE_OUT_PIN_PWM           5
//Depends on the configured rate setting
#define  BOTTLE_IN_DEBOUNCE_TIME      3
#define  BOTTLE_FILL_SPEED            1
#define  BOTTLE_STOP_SPEED            0
#define  BOTTLE_IN_PIN_STOP           7
#define  BOTTLE_IN_PIN_MODE           8
#define  BOTTLE_IN_PIN_FILL           9

/*--[ Types ]--------------------------------------------------------------------------------------------------------------------*/
enum BottleButtons
{
  BOTTLE_IN_STOP = 0,
  BOTTLE_IN_MODE,
  BOTTLE_IN_FILL,
  MAX_BOTTLE_BUTTONS
};

/*--[ Prototypes ]---------------------------------------------------------------------------------------------------------------*/
void BottleStatesInitialise();
void BottleStatesExecute();

/*EOF============================================================================================================================*/
#endif
