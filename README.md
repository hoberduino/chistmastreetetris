# chistmastreetetris
Simple Tetris and more code for FastLED on christmas tree

SW consists of WS2812B LED display options in two configurations as well as three games with a menu and a calibration utility.

SW was originally designed to run on an Arduino ATMega265, reading inputs from an NES controller, outputting to both a 22x22 WS2812B display,
and a christmas tree consisting of multiple strands of WS2812B LEDs hung in a spiral formation (starting at bottom right, going left and up).

Wiring description is below:

/* Xmas Tree Spiral LEDs */
#define NUM_TREE_LEDS    550

/* 22x22 Snaked Display */
#define NUM_LEDS    484

//===============================================================================
//  Pin Declarations
//===============================================================================
//Inputs:
#define NES_DATA          4    // The data pin for the NES controller

/* use 560 ohm pull down resistors (to ground) for RESET_SWITCH_IN and POWER_SWITCH_IN */
/* use ~2k ohm resistor connecting ground to white power/reset wire */
/* The NES reset button is a "momentary" push-button switch. That is, you push it in and the circuit is completed only while you hold it down, 
 * and when released it pops back out again, breaking the circuit. The NES power button is a "latching" push-button switch by default. 
 * That is, you push it in and it locks, completing the circuit as long as it's latched in the "in" position. 
 * You push it again and it pops back out, breaking the circuit. This may have worked nicely for the NES but won't work for a PC. 
 * To change it to a momentary switch I had to remove a little copper tab and staple shaped pin from the top. Pretty simple.
 */
/* use 10k ohm pull up resistor (to 5V) for controller NES_DATA */

//The wiring of the NES switches is as follows:
//Power = Red + Brown
//Reset = Orange + Yellow
//LED = Red + White

/* Do I really need RESET_SWITCH_OUT and POWER_SWITCH_OUT or can I connect to 5V? */
/* Outputs: */
#define NES_CLOCK          2    // The clock pin for the NES controller
#define NES_LATCH          3    // The latch pin for the NES controller
#define LED_PIN            5
#define LED_TREE_PIN       6
#define RESET_SWITCH_IN    8
#define RESET_SWITCH_OUT   9
#define POWER_SWITCH_IN   10
#define POWER_SWITCH_OUT  11
#define MUSIC_PIN_TETRIS (22)
#define MUSIC_PIN_PAC    (24)
#define MUSIC_PIN_MARIO  (26)


NES Controller:
GND -> GND
VCC -> +5V
CLK -> Pin 2 (NES_CLOCK)
Latch -> Pin 3 (NES_LATCH)
Data -> O -> Pin 4          (Board then split with 1k resistor to ground, input to pin 4 (NES_DATA))
	    |
		---> 1k resistor -> GND



