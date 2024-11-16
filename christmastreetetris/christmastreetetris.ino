#include <FastLED.h>
#include <EEPROM.h>

#include <avr/pgmspace.h>


/*  Display everything on a 22x22 board, top left is (0,0)
 *  Convert board to 22x22 snaked display, 20x10 snaked display, 
 *  or xmas tree as dictated by #defines below
 *  Currently outputs both 22x22 snaked display and xmas tree simultaneously
 */

/* Xmas Tree Spiral LEDs */
#define NUM_TREE_LEDS    650

/* 22x22 Snaked Display */
#define NUM_LEDS    484

/* CONTROL MODES */
#define DISP_LIGHT     0
#define DISP_GR        1
#define DISP_RAINBOW   2
#define DISP_JUGGLE    3
#define DISP_CASTLE    4
#define DISP_ONE_COLOR 5
#define DISP_CAL_BOARD 6
#define DISP_VORTEX    7
#define UNUSED_MODE    8
#define DISP_VORTEX_3  9
#define DISP_ROWS_MOVE 10
#define DISP_SPIRAL_1  11
#define DISP_SPIRAL_2  12
#define NUM_DISP_MODES 13



#define BRIGHTNESS  38
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB



/* Number of rows, cols on the Big Display */
#define NUM_DISP_ROWS 22 
#define NUM_DISP_COLS 22

/* Initial defaults overwritten by EEPROM values from Calibration */
/* Max number of display rows, cols for xmas tree */
#define NUM_DISP_ROWS_TREE 22 /*middle twenty are visible */
// NUM_DISP_ROWS 20 for full tetris
#define NUM_DISP_COLS_TREE 20 /* middle ten are visible for tetris */

#define NUM_DISP_ROWS_TETRIS 20 /*middle twenty are visible */
// NUM_DISP_ROWS 20 for full tetris
#define NUM_DISP_COLS_TETRIS 10 /* middle ten are visible */

#define NUM_DISP_ROWS_MENU 20 /*middle twenty are visible */
// NUM_DISP_ROWS 20 for full tetris
#define NUM_DISP_COLS_MENU 12 /* middle ten are visible */

#define NUM_ROWS_MARIO_RUN 18
#define NUM_COLS_MARIO_RUN 17 

#define NUM_GIMKIT_ROWS 256
#define NUM_GIMKIT_COLS 256


#define DISP_COLOR_BLACK        0
#define DISP_COLOR_BLUE         1
#define DISP_COLOR_ORANGE       2
#define DISP_COLOR_YELLOW       3
#define DISP_COLOR_RED          4
#define DISP_COLOR_GREEN        5
#define DISP_COLOR_CYAN         6
#define DISP_COLOR_PEACH        7
#define DISP_COLOR_PURPLE       8
#define DISP_COLOR_LIGHT_BLUE  10 //0x002332, 0x001219
#define DISP_COLOR_WHITE       11
#define DISP_COLOR_GRAY        12
#define DISP_COLOR_BROWN       13
#define DISP_COLOR_PAC_BLUE    14
#define DISP_COLOR_HALF_WHITE  15
#define DISP_COLOR_HALF_BLUE   16
#define DISP_COLOR_HALF_RED    17
#define DISP_COLOR_PAC_DOT     18
#define DISP_COLOR_PINK        19
#define DISP_COLOR_AQUA        20
#define DISP_COLOR_BUSH_GREEN  21
#define DISP_COLOR_HILL_GREEN  22
#define DISP_COLOR_PIPE_GREEN  23
#define DISP_COLOR_Q_YELLOW    24
#define DISP_COLOR_Q_ORANGE    25
#define DISP_COLOR_LIGHT_PINK  26
#define DISP_COLOR_GOLD        27
#define NUM_DISP_COLORS        28

const CRGB numToColor[NUM_DISP_COLORS] = 
{CRGB::Black, CRGB::Blue, 0xff4500, 0xffd700, CRGB::Red, CRGB::Green, CRGB::Cyan, CRGB::PeachPuff,
 0xcf00ff, 0xD7FF00, 0x00090C, CRGB::White, CRGB::Gray, 0x2F1010, 0x0000A0, 0x808080, 
 0x000080, 0x800000, 0x404040, 0xff0080, 0x00cccc, 0x408820, 0x124012, 0x008000, 
 0x888800, 0xFF6500, 0x7F0040, 0xDA9100};
 const CRGB numToColorTree[NUM_DISP_COLORS] = 
{CRGB::Black, CRGB::Blue, 0x45ff00, 0xd7ff00, CRGB::Green, CRGB::Red, 0xff00ff, 0xdaffb9,
 0x00cfff, 0xFFD700, 0x230032, CRGB::White, CRGB::Gray, 0x102F10, 0x0000A0, 0x808080, 
 0x000080, 0x008000, 0x404040, 0x00ff80, 0xcc00cc, 0x884020, 0x401212, 0x800000, 
 0x888800, 0x65FF00, 0x007F40, 0x91DA00};

/* Tetris stuff */
#define NUM_ROWS      24 /*middle twenty are visible */
#define NUM_COLS      16 /* middle ten are visible */
#define PIECE_LENGTH  4
#define NUM_SHAPES 7
#define NUM_TURNS 7
#define ROW 0
#define COL 1
#define MOVE_NONE 0
#define MOVE_LEFT 1
#define MOVE_RIGHT 2
#define MOVE_ROTATE_LEFT 3
#define MOVE_ROTATE_RIGHT 4
#define MOVE_HARD_DROP 5
#define MOVE_DOWN 6
#define MOVE_UP 7
#define MOVE_START 8
#define MOVE_SELECT 9

#define EMPTY_BOARD_ROW 0xE007 /* middle ten columns are visible */
#define NUM_LEVELS 10

#define NUM_PAINT_COLORS 12

/* constants for NES controller inputs */
const int A_BUTTON         = 0;
const int B_BUTTON         = 1;
const int SELECT_BUTTON    = 2;
const int START_BUTTON     = 3;
const int UP_BUTTON        = 4;
const int DOWN_BUTTON      = 5;
const int LEFT_BUTTON      = 6;
const int RIGHT_BUTTON     = 7;



//===============================================================================
//  Pin Declarations
//===============================================================================
//Inputs:
#define NES_DATA          22    // The data pin for the NES controller

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
#define NES_CLOCK          26    // The clock pin for the NES controller
#define NES_LATCH          30    // The latch pin for the NES controller
//#define PICTURE_INPUT_1    34
//#define PICTURE_INPUT_2    38
#define LED_PIN            48
#define LED_TREE_PIN       52
//#define RESET_SWITCH_IN    8
//#define RESET_SWITCH_OUT   9
//#define POWER_SWITCH_IN   10
//#define POWER_SWITCH_OUT  11
//#define MUSIC_PIN_TETRIS (22)
//#define MUSIC_PIN_PAC    (24)
//#define MUSIC_PIN_MARIO  (26)





/* Num display rows and cols for xmas tree */
/* initial values, overwritten by values from EEPROM */
//unsigned char num_tree_rows = 20;//NUM_DISP_ROWS_TREE;
//unsigned char num_tree_cols = 18;//NUM_DISP_COLS_TREE;

/* 0th entry is num lights before start of first row */
 unsigned char ledsInRows[NUM_DISP_ROWS_TREE + 1] =
{0,
 49,45,45,43,38, // GOOD
 36,35,34,31,31, // GOOD
 27,29,27,27,27, // GOOD
 27,25,22,19,17, // GOOD
 10,6};


/* Big LED Display Board (22x22) */
/* Starts with uppermost row, colors from 1:NUM_DISP_COLORS */
unsigned char bigDispBoard[NUM_DISP_ROWS][NUM_DISP_COLS] =
{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};

/* Tetris Start Screen Display */
const unsigned char PROGMEM tetrisStartDisp[NUM_DISP_ROWS_TETRIS][NUM_DISP_COLS_TETRIS] =
  {{6,6,6,4,4,4,8,8,8,0},
   {0,6,0,4,0,0,0,8,0,0},
   {0,6,0,4,4,4,0,8,0,0},
   {0,6,0,4,0,0,0,8,0,0},
   {0,6,0,4,4,4,0,8,0,0},
   {0,0,0,0,0,0,0,0,0,0},
   {3,3,3,5,5,5,1,1,1,0},
   {3,0,3,0,5,0,1,0,0,0},
   {3,3,0,0,5,0,1,1,1,0},
   {3,3,0,0,5,0,0,0,1,0},
   {3,0,3,5,5,5,1,1,1,0},{0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0}};


/* Used for Twinkling Stars Effect */
/* Base background color */
#define BASE_COLOR       CRGB(32,0,32)

// Peak color to twinkle up to
#define PEAK_COLOR       CRGB(64,0,64)


// Currently set to brighten up a bit faster than it dims down, 
// but this can be adjusted.

// Amount to increment the color by each loop as it gets brighter:
#define DELTA_COLOR_UP   CRGB(4,0,4)

// Amount to decrement the color by each loop as it gets dimmer:
#define DELTA_COLOR_DOWN CRGB(1,0,1)


// Chance of each pixel starting to brighten up.  
// 1 or 2 = a few brightening pixels at a time.
// 10 = lots of pixels brightening at a time.
#define CHANCE_OF_TWINKLE 50


#define PAINT_COLOR_BLACK       11
#define PAINT_COLOR_BLUE         0
#define PAINT_COLOR_ORANGE       1
#define PAINT_COLOR_YELLOW       2
#define PAINT_COLOR_RED          3
#define PAINT_COLOR_GREEN        4
#define PAINT_COLOR_CYAN         5
#define PAINT_COLOR_PURPLE       6
#define PAINT_COLOR_GOLD         7
#define PAINT_COLOR_WHITE        8
#define PAINT_COLOR_PINK         9
#define PAINT_COLOR_AQUA        10

const unsigned char PROGMEM paintColors[NUM_PAINT_COLORS] = 
 {DISP_COLOR_BLUE, DISP_COLOR_ORANGE, DISP_COLOR_YELLOW,
  DISP_COLOR_RED, DISP_COLOR_GREEN, DISP_COLOR_CYAN,
  DISP_COLOR_PURPLE, DISP_COLOR_GOLD, DISP_COLOR_WHITE,
  DISP_COLOR_PINK, DISP_COLOR_AQUA, DISP_COLOR_BLACK};


/* Tetris Game Board */
unsigned long gameBoard[NUM_ROWS] =
{EMPTY_BOARD_ROW, EMPTY_BOARD_ROW, EMPTY_BOARD_ROW, EMPTY_BOARD_ROW, 
 EMPTY_BOARD_ROW, EMPTY_BOARD_ROW, EMPTY_BOARD_ROW, EMPTY_BOARD_ROW, 
 EMPTY_BOARD_ROW, EMPTY_BOARD_ROW, EMPTY_BOARD_ROW, EMPTY_BOARD_ROW,
 EMPTY_BOARD_ROW, EMPTY_BOARD_ROW, EMPTY_BOARD_ROW, EMPTY_BOARD_ROW,
 EMPTY_BOARD_ROW, EMPTY_BOARD_ROW, EMPTY_BOARD_ROW, EMPTY_BOARD_ROW,
 EMPTY_BOARD_ROW, EMPTY_BOARD_ROW, 0xFFFF, 0xFFFF};

/* Tetris 7 shapes */
/* listed as row, row, row, row (LSB) from top to bottom, left to right */
const unsigned int PROGMEM shapeMove[NUM_SHAPES][NUM_TURNS] = 
{
/* Shape 0 - I */
{0x0F00, 0x2222, 0x00F0, 0x4444},
/* Shape 1 - J */
{0x08E0, 0x0644, 0x00E2, 0x044C},
/* Shape 2 - L */
{0x02E0, 0x0446, 0x00E8, 0x0C44},
/* Shape 3 - O */
{0x0660, 0x0660, 0x0660, 0x0660},
/* Shape 4 - S */
{0x06C0, 0x0462, 0x006C, 0x08C4},
/* Shape 5 - Z */
{0x0C60, 0x0264, 0x00C6, 0x04C8},
/* Shape 6 - T */
{0x04E0, 0x0464, 0x00E4, 0x04C4}};

/* Tetris Next 7 shapes, 99 - none*/
byte next7[NUM_SHAPES] = {99,99,99,99,99,99,99};

/* Tetris current location in next7 */
byte curShapeIter = 0;
/* Tetris current shape rotation */
byte curShapeRot = 0;
/* Tetris current shape active */
bool curShapeActive = false;

/* Tetris count from top left corner (0,0) to (23,15) (row,col) */
int blockLoc[2] = {0, 0};
/* (0,3) to (21,12) are valid playing area, (2,3) to (21,12) are visible */
/* (0,1) to (20,11) are valid piece locations */

unsigned int firstRow = 99;

// All those LEDs
CRGB leds[NUM_LEDS  + NUM_TREE_LEDS];
//CRGB leds_tree[NUM_LEDS];

/* Number of display rows on tree */
#define NUM_ROWS_EEPROM_ADDRESS 0 
/* Number of display cols on tree */
#define NUM_COLS_EEPROM_ADDRESS 1 
/* Number of LEDs before given row on tree */
/* Max number of 22 rows (NUM_DISP_ROWS) */
#define ROW_OFFSET_EEPROM_ADDRESS 2 

#define MAX_LED_OFFSET 50



void setup() {
  
  delay( 3000 ); // power-up safety delay
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS + NUM_TREE_LEDS).setCorrection( TypicalLEDStrip );
  //FastLED.addLeds<LED_TYPE, LED_TREE_PIN, COLOR_ORDER>(leds, NUM_TREE_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
  
  Serial.begin(9600);

  /* Change to seed when user presses START */
  randomSeed(analogRead(0));

  // Set appropriate pins to inputs
  pinMode(NES_DATA, INPUT);
//  pinMode(PICTURE_INPUT_1, INPUT);
//  pinMode(PICTURE_INPUT_2, INPUT);
  
  // Set appropriate pins to outputs
  pinMode(NES_CLOCK, OUTPUT);
  pinMode(NES_LATCH, OUTPUT);


  
  // Set initial states
  digitalWrite(NES_CLOCK, LOW);
  digitalWrite(NES_LATCH, LOW);

  /* Read values from EEPROM */
//  unsigned char i = EEPROM.read(NUM_ROWS_EEPROM_ADDRESS); /* num rows to be stored at address 0 */
//  if ((i > 0) && (i <= NUM_DISP_ROWS_TREE))
//    num_tree_rows = i;
//  i = EEPROM.read(NUM_COLS_EEPROM_ADDRESS); /* num cols to be stored at address 0 */
//  if ((i > 0) && (i <= NUM_DISP_COLS_TREE))
//    num_tree_cols = i; 

  /* Update ledsBeforeRows values if something stored there > 0 */
  /* Right now these are stored from top to bottom */
//  for (i = 0; i < NUM_DISP_ROWS_TREE; i++);
//  {
//    unsigned char num_leds = EEPROM.read(ROW_OFFSET_EEPROM_ADDRESS + i);
//    if ((num_leds > 0) && (num_leds < 50))
//      ledsBeforeRows[i] = num_leds;
//  }
  
}




/* Generic Display Functions */




/* Convert from bigDispBoard to LEDs on 22x22 Display */
/* showLed is used in case of Castle Display or similar where only desire to 
 * display non-black pixels and not call FastLED.show()
 */
void displayBigBoardTwoTwo(bool showLed)
{
  unsigned int i, j;

  for(i = 0; i < NUM_LEDS; i++)
    if (showLed == true)
      leds[i] = CRGB::Black;
  
  /* bottom row to top, right to left */
  for(i = 0; i < NUM_DISP_ROWS; i++)
  {
    for(j = 0; j < NUM_DISP_COLS; j++)
    {   
      if ((showLed == true) || (bigDispBoard[i][j] != DISP_COLOR_BLACK))
      {
        if ((i % 2) == 0)
          leds[i * NUM_DISP_COLS + j] = numToColor[bigDispBoard[i][j]];
        else
          leds[i * NUM_DISP_COLS + NUM_DISP_COLS - j - 1] = numToColor[bigDispBoard[i][j]];
      }
    }
  }

  if (showLed == true)
    FastLED.show();
}

/* Convert from bigDispBoard to 20x10 LEDs */
/* showLed is used in case of Castle Display or similar where only desire to 
 * display non-black pixels and not call FastLED.show()
 */
/*void displayLEDBoardTwentyTen(bool showLed)
{
  unsigned int i, j;
  
  for(i = 0; i < NUM_LEDS; i++)
    if (showLed == true)
      leds[i] = CRGB::Black;
  
  // bottom row to top, right to left
  for(i = 0; i < NUM_DISP_ROWS_TETRIS; i++)
  {
    for(j = 0; j < NUM_DISP_COLS_TETRIS; j++)
    {   
      if ((showLed == true) || (bigDispBoard[i + 1][j + 6] != DISP_COLOR_BLACK))
      {
        if ((i % 2) == 0)
          leds[i * NUM_DISP_COLS_TETRIS + j] = numToColor[bigDispBoard[i + 1][j + 6]];
        else
          leds[i * NUM_DISP_COLS_TETRIS + NUM_DISP_COLS_TETRIS - j - 1] = numToColor[bigDispBoard[i + 1][j + 6]];
      }
    }
  }

  if (showLed == true)
    FastLED.show();
}
*/

// TREE (Start at bottom right and wind around the tree upward)
// Row  13: BeforeLEDs, leftmost, ..., rightmost, 
// Row ...: BeforeLEDs, leftmost, ..., rightmost, 
// Row   1: BeforeLEDs(50-59), leftmost(49), ..., rightmost(40), 
// Row   0: BeforeLEDs(20-29), leftmost(19), ..., rightmost(10), 

// LED Display Board (start at top left and snake down board)
// Row  0: leftmost(0), .., rightmost(9)
// Row  1: leftmost(19), .., rightmost(10)
// ...
// Row 18: leftmost(180), .., rightmost(189)
// Row 19: leftmost(199), .., rightmost(190)

/* Convert from bigDispBoard to LEDs on Tree */
/* Start at bottom, right and wind around tree */
void displayLEDTree(bool showLed, bool dispCastle)
{
  unsigned int currentLED = NUM_LEDS; // start after display board of LEDs
  unsigned char num_disp_cols = NUM_DISP_COLS_TREE;
  unsigned char first_disp_col = 0;
  
  /* bottom row to top, right to left */
  for(int i = 0; i < NUM_DISP_ROWS_TREE; i++)
  {
    unsigned int num_leds_current_row = ledsInRows[i + 1];
    if (num_leds_current_row < NUM_DISP_COLS_TREE)
    {
       num_disp_cols = num_leds_current_row;
       first_disp_col = (NUM_DISP_COLS_TREE - num_disp_cols) / 2;
    }

    /* starts at bottom row of tree, right to left, rightmost display col */
    for(int j = num_disp_cols - 1; j >= first_disp_col; j--)
    {      
      if (((dispCastle == true) and (bigDispBoard[(NUM_DISP_ROWS_TREE - i - 1)][j] > 0)) ||
         ((showLed == true) and (dispCastle == false)))
         leds[currentLED] = numToColorTree[bigDispBoard[(NUM_DISP_ROWS_TREE - i - 1)][j]];
      currentLED++;
    }

    /* clear remaining leds in row */
    for(int j = num_disp_cols; j < num_leds_current_row; j++)
    {      
      if ((showLed == true) and (dispCastle == false))
        leds[currentLED] = CRGB::Black;
      currentLED++;
    }

  }

  for(int i = currentLED; i < (NUM_LEDS + NUM_TREE_LEDS); i++)
    if ((showLed == true) and (dispCastle == false))
    {
      leds[currentLED] = CRGB::Black;
      currentLED++;
    }

  if (showLed == true)
    FastLED.show();
}

/* Switch between displaying LED Board and LED Tree */
void displayLEDs(bool showLed, bool dispCastle)
{
    displayBigBoardTwoTwo(showLed);
    displayLEDTree(showLed, dispCastle);
//    displayLEDBoardTwentyTen(showLed);
}

// Descriptions of pixel representations of digits 0 - 9
unsigned const char PROGMEM left_column [10] =   {0x1F, 0x00, 0x1D, 0x15, 0x07, 0x17, 0x1F, 0x01, 0x1F, 0x07}; // top is LSB, bottom is MSB
unsigned const char PROGMEM middle_column [10] = {0x11, 0x00, 0x15, 0x15, 0x04, 0x15, 0x15, 0x01, 0x15, 0x05}; // top is LSB, bottom is MSB
unsigned const char PROGMEM right_column [10] =  {0x1F, 0x1F, 0x17, 0x1F, 0x1F, 0x1D, 0x1D, 0x1F, 0x1F, 0x1F}; // top is LSB, bottom is MSB

/* Generic Number Display (0-199) */
void displayScore(unsigned int totalScore, bool show_leds)
{
  unsigned int j;
  unsigned int tens_digit = (totalScore % 100) / 10;
  unsigned int ones_digit = totalScore % 10;

  /* Display Score (number of lines) */
  for(j = 0; j < 5; j++) // for each column
  {
    if (totalScore > 99)
      bigDispBoard[14+j][7] = 5; // Green
      
    if (((pgm_read_byte(&left_column[tens_digit]) >> j) & 0x1) > 0)
      bigDispBoard[14+j][9] = 4; // Red
    if (((pgm_read_byte(&middle_column[tens_digit]) >> j) & 0x1) > 0)
      bigDispBoard[14+j][10] = 4; // Red
    if (((pgm_read_byte(&right_column[tens_digit]) >> j) & 0x1) > 0)
      bigDispBoard[14+j][11] = 4; // Red
    if (((pgm_read_byte(&left_column[ones_digit]) >> j) & 0x1) > 0)
      bigDispBoard[14+j][13] = 5; // Green
    if (((pgm_read_byte(&middle_column[ones_digit]) >> j) & 0x1) > 0)
      bigDispBoard[14+j][14] = 5; // Green
    if (((pgm_read_byte(&right_column[ones_digit]) >> j) & 0x1) > 0)
      bigDispBoard[14+j][15] = 5; // Green 
  }
     
  if (show_leds)
    displayLEDs(true, false);
}




/* Controller Functions */



byte readNesController() 
{  
  // Pre-load a variable with all 1's which assumes all buttons are not
  // pressed. But while we cycle through the bits, if we detect a LOW, which is
  // a 0, we clear that bit. In the end, we find all the buttons states at once.
  int tempData = 255;
    
  // Quickly pulse the NES_LATCH pin so that the register grab what it see on
  // its parallel data pins.
  digitalWrite(NES_LATCH, HIGH);
  digitalWrite(NES_LATCH, LOW);
 
  // Upon latching, the first bit is available to look at, which is the state
  // of the A button. We see if it is low, and if it is, we clear out variable's
  // first bit to indicate this is so.
  if (digitalRead(NES_DATA) == LOW)
    bitClear(tempData, A_BUTTON);
    
  // Clock the next bit which is the B button and determine its state just like
  // we did above.
  digitalWrite(NES_CLOCK, HIGH);
  digitalWrite(NES_CLOCK, LOW);
  if (digitalRead(NES_DATA) == LOW)
    bitClear(tempData, B_BUTTON);
  
  // Now do this for the rest of them!
  
  // Select button
  digitalWrite(NES_CLOCK, HIGH);
  digitalWrite(NES_CLOCK, LOW);
  if (digitalRead(NES_DATA) == LOW)
    bitClear(tempData, SELECT_BUTTON);

  // Start button
  digitalWrite(NES_CLOCK, HIGH);
  digitalWrite(NES_CLOCK, LOW);
  if (digitalRead(NES_DATA) == LOW)
    bitClear(tempData, START_BUTTON);

  // Up button
  digitalWrite(NES_CLOCK, HIGH);
  digitalWrite(NES_CLOCK, LOW);
  if (digitalRead(NES_DATA) == LOW)
    bitClear(tempData, UP_BUTTON);
    
  // Down button
  digitalWrite(NES_CLOCK, HIGH);
  digitalWrite(NES_CLOCK, LOW);
  if (digitalRead(NES_DATA) == LOW)
    bitClear(tempData, DOWN_BUTTON);

  // Left button
  digitalWrite(NES_CLOCK, HIGH);
  digitalWrite(NES_CLOCK, LOW);
  if (digitalRead(NES_DATA) == LOW)
    bitClear(tempData, LEFT_BUTTON);  
    
  // Right button
  digitalWrite(NES_CLOCK, HIGH);
  digitalWrite(NES_CLOCK, LOW);
  if (digitalRead(NES_DATA) == LOW)
    bitClear(tempData, RIGHT_BUTTON);
    
  // After all of this, we now have our variable all bundled up
  // with all of the NES button states.*/
  return tempData;
}

unsigned int getMove() {

  byte nesRegister  = 0;    // We will use this to hold current button states
  unsigned long lastMoveTime = 0;
  unsigned long currentMillis = 0;
  unsigned int returnMove = MOVE_NONE;

  static bool lastMoveStart = false;
  
  // This function call will return the states of all NES controller's register
  // in a nice 8 bit variable format. Remember to refer to the table and
  // constants above for which button maps where!
  nesRegister = readNesController();
  
  // Slight delay before we debug what was pressed so we don't spam the
  // serial monitor.
  //delay(50);
  
  // To give you an idea on how to use this data to control things for your
  // next project, look through the serial terminal code below. Basically,
  // just choose a bit to look at and decide what to do whether HIGH (not pushed)
  // or LOW (pushed). What is nice about this test code is that we mapped all
  // of the bits to the actual button name so no useless memorizing!

  currentMillis = millis();

  
  if (bitRead(nesRegister, START_BUTTON) == 0)
    returnMove = MOVE_START;
  
  if (bitRead(nesRegister, SELECT_BUTTON) == 0)
    returnMove = MOVE_SELECT;
    
  if (bitRead(nesRegister, UP_BUTTON) == 0)
    returnMove = MOVE_UP;
    
  if (bitRead(nesRegister, DOWN_BUTTON) == 0)
    returnMove = MOVE_DOWN;
    
  if (bitRead(nesRegister, LEFT_BUTTON) == 0)
    returnMove = MOVE_LEFT;  
  
  if (bitRead(nesRegister, RIGHT_BUTTON) == 0)
    returnMove = MOVE_RIGHT;

  if (bitRead(nesRegister, A_BUTTON) == 0)
    returnMove = returnMove | (MOVE_ROTATE_RIGHT << 4);
    
  if (bitRead(nesRegister, B_BUTTON) == 0)
    returnMove = returnMove | (MOVE_ROTATE_LEFT << 4);

  if (returnMove == MOVE_START)
  {
    if ((lastMoveStart == false) ||
       ((lastMoveStart == true) && (abs(currentMillis - lastMoveTime) > 1000)))
    {
       lastMoveStart = true;
       lastMoveTime = currentMillis;
    }
    else
      returnMove = MOVE_NONE;   
  }
  else if (returnMove == MOVE_SELECT)
  {
    if (abs(currentMillis - lastMoveTime) > 300)
      lastMoveTime = currentMillis;
    else
      returnMove = MOVE_NONE;
  }
  else if (returnMove != MOVE_NONE)
  {
    if ((abs(currentMillis - lastMoveTime)) > 100)
      lastMoveTime = currentMillis;
    else
      returnMove = MOVE_NONE;
  }

  return returnMove;

}




/* TETRIS Display Functions */


/*
void displayGameBoardSerial()
{
  int i, j;
  unsigned int curShape = pgm_read_word_near(&shapeMove[next7[curShapeIter]][curShapeRot]);
  unsigned int rightAlignedPieceRow = 0;
  
  unsigned long displayBoard[PIECE_LENGTH] = {0,0,0,0};

  for(i = PIECE_LENGTH - 1; i >= 0; i--)
  {
    rightAlignedPieceRow = ((curShape & (0xF << (4*i))) >> (4*i));
    displayBoard[3 - i] |= rightAlignedPieceRow << (12 - blockLoc[COL]);    
  }
  Serial.println();
  Serial.println();

  for(i = 2; i < NUM_ROWS - 2; i++)
  {
    for(j = 12; j > 2; j--)
    {
      if ((gameBoard[i] & (1 << j)) || (curShapeActive &&
          ((i >= blockLoc[ROW] && (i < (blockLoc[ROW] + 4))) &&
           (displayBoard[i - blockLoc[0]] & (1 << j)))))
        Serial.print("*");
      else
        Serial.print("-");
    }
    Serial.println();
  }
}
*/


void displayFullRow(int rowNum)
{
  unsigned int j, ledNum;
  const unsigned int unused_rows_top = (NUM_DISP_ROWS - NUM_DISP_ROWS_TETRIS) / 2;
  const unsigned int unused_cols_left = (NUM_DISP_COLS - NUM_DISP_COLS_TETRIS) / 2;
  
  if (rowNum > 1)
  {
    for(j = 0; j < 10; j++)
    {
      if ((j % 2) == 0)
        bigDispBoard[rowNum - 2 + unused_rows_top][j + unused_cols_left] = DISP_COLOR_RED;
      else
        bigDispBoard[rowNum - 2 + unused_rows_top][j + unused_cols_left] = DISP_COLOR_GREEN;
    }
  }
  displayLEDs(true, false);
}

void displayTetrisStart()
{
  unsigned int unused_rows_top = (NUM_DISP_ROWS - NUM_DISP_ROWS_TETRIS) / 2;
  unsigned int unused_cols_left = (NUM_DISP_COLS - NUM_DISP_COLS_TETRIS) / 2;
  
  // TETRIS Splash Screen
  unsigned int i, j;
  for(i = 0; i < NUM_DISP_ROWS_TETRIS; i++)
    for(j = 0; j < NUM_DISP_COLS_TETRIS; j++)
      bigDispBoard[i + unused_rows_top][j + unused_cols_left] = pgm_read_byte_near(&tetrisStartDisp[i][j]);

  displayLEDs(true, false);
}

const unsigned int PROGMEM shapeToColor[NUM_SHAPES] = 
   {DISP_COLOR_PURPLE, DISP_COLOR_BLUE, DISP_COLOR_ORANGE, DISP_COLOR_YELLOW,
    DISP_COLOR_RED, DISP_COLOR_GREEN, DISP_COLOR_CYAN};

void displayGameBoard(bool slow)
{
  //displayGameBoardSerial();

  int i, j;
  unsigned int curShape = pgm_read_word_near(&shapeMove[next7[curShapeIter]][curShapeRot]);
  unsigned int rightAlignedPieceRow = 0;
  bool somethingThisRow = false;
  unsigned int shape_color = 0;
  
  unsigned long displayPieceBoard[PIECE_LENGTH] = {0,0,0,0};

  const unsigned int unused_rows_top = (NUM_DISP_ROWS - NUM_DISP_ROWS_TETRIS) / 2;
  const unsigned int unused_cols_left = (NUM_DISP_COLS - NUM_DISP_COLS_TETRIS) / 2;

  /* Clear top row of bigDispBoard */
  for(j = 0; j < NUM_DISP_ROWS; j++)
    bigDispBoard[0][j] = DISP_COLOR_BLACK;
  

  for(i = PIECE_LENGTH - 1; i >= 0; i--)
  {
    rightAlignedPieceRow = ((curShape & (0xF << (4*i))) >> (4*i));
    displayPieceBoard[3 - i] |= rightAlignedPieceRow << (12 - blockLoc[COL]);    
  }

  /* top row to bottom, left to right */
  for(i = NUM_ROWS - 3; i > 1; i--)
  {
    for(j = 12; j > 2; j--)
    {      
      if (gameBoard[i] & (1 << j))
      {
        /* LEDS snake from 1 row to next */
        bigDispBoard[i - 2 + unused_rows_top][12 - j + unused_cols_left] = DISP_COLOR_PEACH;
        somethingThisRow = true;
      }
      else
      {
        bigDispBoard[i - 2 + unused_rows_top][12 - j + unused_cols_left] = DISP_COLOR_BLACK;
      }
    }
    if (((slow == true) && (somethingThisRow == true)) && (i <= firstRow))
    {
      displayLEDs(true, false);
      delay(100);
      somethingThisRow = false;
    }
  }
  firstRow = 99;

  shape_color = pgm_read_byte_near(&shapeToColor[next7[curShapeIter]]);

  for(i = blockLoc[ROW]; i < blockLoc[ROW] + 4; i++)
  {
    /* left to right: 12 leftmost on gameBoard, 0 on leds row */
    for(j = 12; j > 2; j--)
    {
      if (curShapeActive && ((i >= blockLoc[ROW] && (i < (blockLoc[ROW] + 4))) &&
           (displayPieceBoard[i - blockLoc[0]] & (1 << j))))
        bigDispBoard[i - 2 + unused_rows_top][12 - j + unused_cols_left] = shape_color;
    }
  }

  displayLEDs(true, false);
}



/* TETRIS Play Functions */




void init_tetris()
{
  unsigned int i, j;
  for(i = 0; i < NUM_DISP_ROWS; i++)
    for(j = 0; j < NUM_DISP_COLS; j++)
      bigDispBoard[i][j] = DISP_COLOR_BLACK;
      
   // Set initial states
  digitalWrite(NES_CLOCK, LOW);
  digitalWrite(NES_LATCH, LOW);

  for(i = 0; i < NUM_ROWS - 2; i++)
    gameBoard[i] = EMPTY_BOARD_ROW;
  gameBoard[NUM_ROWS - 2] = 0xFFFF;
  gameBoard[NUM_ROWS - 1] = 0xFFFF;

  for(i = 0; i < NUM_SHAPES; i++)
    next7[i] = 99;

  curShapeIter = NUM_SHAPES;
  curShapeRot = 0;
  curShapeActive = false;

  blockLoc[0] = 0;
  blockLoc[1] = 0;
  //digitalWrite(MUSIC_PIN_TETRIS, HIGH);
  randomSeed(millis());
  displayTetrisStart();
  delay(3000);
}

void random_bag() {
  int i, j, place, adj_place;
  for(i = 0; i < NUM_SHAPES; i++)
    next7[i] = 99;
  /* randomize grab bag */
  for(i = 0; i < NUM_SHAPES; i++)
  {
    place = random(NUM_SHAPES);
    for(j = 0; j < NUM_SHAPES; j++)
    {
      adj_place = (place + j) % NUM_SHAPES;
      if (next7[adj_place] == 99)
      {
        next7[adj_place] = i;
        break;
      }
    }
  }  
}

byte updateGameBoard()
{
  int i, j;
  byte numLines = 0;
  unsigned int newShape = pgm_read_word_near(&shapeMove[next7[curShapeIter]][curShapeRot]);
  unsigned int rightAlignedPieceRow = 0;
  for(i = PIECE_LENGTH - 1; i >= 0; i--)
  {
    rightAlignedPieceRow = ((newShape & (0xF << (4*i))) >> (4*i));
    gameBoard[blockLoc[ROW] + 3 - i] |= rightAlignedPieceRow << (12 - blockLoc[COL]);   
  }

  /* Check for complete rows */
  for(i = 0; i < NUM_ROWS - 2; i++)
  {
    if ((gameBoard[i] & 0xFFFF) == 0xFFFF)
    {
      displayFullRow(i);
      numLines++;
      if (firstRow == 99)
        firstRow = i;
    }
  }
  if (numLines > 0)
    delay(250);

  /* Check for complete rows */
  for(i = 0; i < NUM_ROWS - 2; i++)
  {
    if ((gameBoard[i] & 0xFFFF) == 0xFFFF)
    {
      for(j = i; j > 0; j--)
        gameBoard[j] = gameBoard[j-1];
      gameBoard[0] = EMPTY_BOARD_ROW;
    }
  }
    
  return numLines;
}

bool canMove(byte moveDir)
{
  int i;
  unsigned int rightAlignedPieceRow = 0;
  bool moveOK = true;
  byte shapeRot = curShapeRot;
  unsigned int newShape;
  int latMove = 0;
  byte dropRow = 0;
  if (moveDir == MOVE_LEFT)
    latMove = -1;
  else if (moveDir == MOVE_RIGHT)
    latMove = 1;
  else if (moveDir == MOVE_ROTATE_LEFT)
    shapeRot = (curShapeRot + 4 - 1) % 4;
  else if (moveDir == MOVE_ROTATE_RIGHT)
    shapeRot = (curShapeRot + 1) % 4;
  else if (moveDir == MOVE_DOWN)
  {
    dropRow = 1;
  }
  else
    return false;

  newShape = pgm_read_word_near(&shapeMove[next7[curShapeIter]][shapeRot]);

  /* Compare each constructed row of piece to complete row of gameboard */
  for(i = PIECE_LENGTH - 1; i >= 0; i--)
  {
    rightAlignedPieceRow = ((newShape & (0xF << (4*i))) >> (4*i));
    
    if (((rightAlignedPieceRow << (12 - blockLoc[COL] - latMove)) & gameBoard[blockLoc[ROW] + (3 - i) + dropRow]) > 0)
    {
      moveOK = false;
      break;
    }
  }
  return moveOK;
}

void movePiece(byte moveDir)
{
  if (moveDir == MOVE_LEFT)
    blockLoc[COL] = blockLoc[COL] - 1;
  else if (moveDir == MOVE_RIGHT)
    blockLoc[COL] = blockLoc[COL] + 1;
  else if (moveDir == MOVE_ROTATE_LEFT)
    curShapeRot = (curShapeRot + 4 - 1) % 4;
  else if (moveDir == MOVE_ROTATE_RIGHT)
    curShapeRot = (curShapeRot + 1) % 4;
  else if (moveDir == MOVE_DOWN)
    blockLoc[ROW] = blockLoc[ROW] + 1;
}

bool attemptMovePiece(unsigned int moveDir)
{
  byte tempMoveDir = moveDir & 0xF;
  if ((tempMoveDir != MOVE_HARD_DROP) && (tempMoveDir != MOVE_NONE))
    if (canMove(tempMoveDir))
      movePiece(tempMoveDir);
    else if (tempMoveDir == MOVE_DOWN)
      return false;
      
  //else if (moveDir == MOVE_HARD_DROP)
  //  while(1)
  //  {
  //    if (canMove(moveDir))
  //      movePiece(moveDir);
  //    else
  //      break;
  //  }

  tempMoveDir = (moveDir >> 4) & 0xF;
  if (tempMoveDir != MOVE_NONE)
    if (canMove(tempMoveDir))
      movePiece(tempMoveDir);

  return true;
}

/* Tetris Time spent per row by level */
const unsigned int PROGMEM millisPerRow[NUM_LEVELS] = 
  {1000, 793, 618, 473, 355, 262, 190, 135, 94, 64};

void play_tetris()
{
  byte numNewLines = 0;
  bool gameEnd = false;
  //bool playMusic = false;
  unsigned long timePieceStartRow;
  unsigned int moveDir = MOVE_NONE;
  unsigned long currentMillis = 0;
  unsigned int totalScore = 0;

  /* Generate Random Bag */
  random_bag();

  /* Tetris Current game level, 0 - 9 */
  byte gameLevel = 0;
  
  init_tetris();

  
  /* If new tetris game, Loop */
  while (gameEnd == false)
  {
    currentMillis = millis();
    /* Check for new bag */
    //Serial.println(curShapeIter);
    //Serial.println(NUM_SHAPES);
    if (curShapeIter == NUM_SHAPES)
    {
      random_bag();
      curShapeIter = 0;
    }
 
    /* Insert new piece */
    if (!curShapeActive)
    {
      blockLoc[ROW] = 0;
      blockLoc[COL] = 6;
      curShapeActive = true;
      timePieceStartRow = currentMillis;
    }

    while ((curShapeActive) && (gameEnd == false))
    {
      currentMillis = millis();
      /* look for movements */
      moveDir = getMove();
      if ((moveDir != MOVE_NONE) && (moveDir != MOVE_START) && (moveDir != MOVE_SELECT))
      {
        attemptMovePiece(moveDir);
        /* Display gameboard */
        displayGameBoard(false);
      }
      else if (moveDir == MOVE_START)
      {
        moveDir = MOVE_NONE;
        while (moveDir != MOVE_START)
        {
          currentMillis = millis();
          moveDir = getMove();
        }
      } 
//      else if (moveDir == MOVE_SELECT)
//      {
//        playMusic = !playMusic;
//        if (playMusic)
//          digitalWrite(MUSIC_PIN_TETRIS, HIGH);
//        else
//          digitalWrite(MUSIC_PIN_TETRIS, LOW);
//      }
    
      /* Check if can drop */
      if ((currentMillis - timePieceStartRow) > pgm_read_word_near(&millisPerRow[gameLevel]))
      {
        if (!attemptMovePiece(MOVE_DOWN))
        {
          numNewLines = updateGameBoard();
          curShapeIter++;
          curShapeActive = false;
          curShapeRot = 0;
          displayGameBoard(numNewLines > 0);
          totalScore += numNewLines;
          gameLevel = totalScore / 10;
        } 
        else
        {
          displayGameBoard(false);
        }
        timePieceStartRow = currentMillis;
      }

      delay(60);
    }
    /* Check for Game End */
    /* TODO Fix to only look at rows 0 and 1 */
    if ((( gameBoard[0] & 0x1FF8) > 0) || (( gameBoard[1] & 0x1FF8) > 0) || (( gameBoard[2] & 0x1FF8) > 0) || (( gameBoard[3] & 0x1FF8) > 0))
      gameEnd = true;

    delay(20);
      
  }
   /* if end of game, display score */ 
   displayTetrisStart();
   displayScore(totalScore, true); 
   delay(3000);
}




/* Lights Functions */



unsigned int light_twinkle = 0;
void display_green_red(unsigned char color1_in, unsigned char color2_in)
{
  int i;
  for(i = NUM_LEDS; i < NUM_LEDS + NUM_TREE_LEDS; i++)
  {
    /* LEDS snake from 1 row to next */
    if (((i + light_twinkle) % 30) < 10)
      leds[i] = numToColorTree[color1_in];
    else if (((i + light_twinkle) % 30) < 20)
      leds[i] = numToColorTree[color2_in];
    else
      leds[i] = CRGB::Black;
  }
  
  light_twinkle = (light_twinkle + 1) % 30;

  FastLED.show();
  delay(20);
}

uint8_t gHue = 0;
void display_rainbow()
{

  fill_rainbow( leds, NUM_LEDS + NUM_TREE_LEDS, gHue, 7);  
  if( random8() < CHANCE_OF_TWINKLE) {
    leds[ random16(NUM_LEDS + NUM_TREE_LEDS) ] += CRGB::White;
  }

  for(int i = 0; i < NUM_LEDS; i++) 
     leds[i] = CRGB::Black;
     
  FastLED.show();
  delay(20);
  gHue+=5; // slowly cycle the "base color" through the rainbow
}


void display_lights()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS + NUM_TREE_LEDS, 10);
  int pos;
  for(int i = 0; i < 10; i++)
  {
     pos = random16(NUM_LEDS + NUM_TREE_LEDS);
     leds[pos] += CHSV( gHue + random8(64), 200, 255);
     gHue++;
  }

  for(int i = 0; i < NUM_LEDS; i++) 
     leds[i] = CRGB::Black;
  
  FastLED.show();
  delay(50);
}

void display_one_color()
{
  for(int i = NUM_LEDS; i < NUM_LEDS + NUM_TREE_LEDS; i++)
    leds[i] = CHSV( gHue, 200, 255);
    
  gHue++;
  FastLED.show();
  delay(100);
}

void display_sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS + NUM_TREE_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS + NUM_TREE_LEDS - 1 );
  leds[pos] += CHSV( gHue, 255, 192);

  for(int i = 0; i < NUM_LEDS; i++) 
     leds[i] = CRGB::Black;

  FastLED.show();
  delay(50);
}

void display_juggle()
{
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS + NUM_TREE_LEDS, 20);

  byte dothue = 0;
  
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS + NUM_TREE_LEDS - 1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }

  for(int i = 0; i < NUM_LEDS; i++) 
     leds[i] = CRGB::Black;

  FastLED.show();
  delay(100);
}

/* Castle Display */
const unsigned char PROGMEM castleDisp[12][NUM_DISP_COLS_TETRIS] =
  {{0,0,0,0,0,0,9,0,0,0},
   {0,0,0,0,0,0,9,0,0,0},
   {0,0,0,0,0,1,11,0,0,0},
   {0,0,0,9,0,11,11,1,0,0},
   {0,0,0,1,0,11,11,11,0,0},
   {0,0,0,11,11,11,11,11,0,0},
   {0,0,0,11,11,11,11,11,0,0},
   {1,0,1,11,10,10,11,1,0,1},
   {12,0,12,11,12,12,11,12,0,12},
   {12,12,12,12,12,12,12,12,12,12},
   {12,12,12,12,0,0,12,12,12,12},
   {12,12,12,12,0,0,12,12,12,12}};

void display_castle()
{
  unsigned int i, j;
  int pos;
  unsigned int unused_rows_top = (NUM_DISP_ROWS - NUM_DISP_ROWS_TETRIS) / 2;
  unsigned int unused_cols_left = (NUM_DISP_COLS - NUM_DISP_COLS_TETRIS) / 2;


  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS + NUM_TREE_LEDS, 10);
  pos = random16(NUM_TREE_LEDS) + NUM_LEDS;
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
  pos = random16(NUM_TREE_LEDS) + NUM_LEDS;
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
  pos = random16(NUM_TREE_LEDS) + NUM_LEDS;
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
  
  for(i = 0; i < NUM_DISP_ROWS; i++)
    for(j = 0; j < NUM_DISP_COLS; j++)
      bigDispBoard[i][j] = DISP_COLOR_BLACK;

  
  for(i = 0; i < 12; i++)
    for(j = 0; j < NUM_DISP_COLS_TETRIS; j++)
      bigDispBoard[i + unused_rows_top + 8][j + unused_cols_left] = pgm_read_byte_near(&castleDisp[i][j]);

  displayLEDTree(true, true);

  delay(20);
}

void display_cal_board()
{
  unsigned int i, j;

  for(i = 0; i < NUM_DISP_ROWS; i++)
    for(j = 0; j < NUM_DISP_COLS; j++)
      bigDispBoard[i][j] = DISP_COLOR_BLACK;

  for(i = 0; i < NUM_DISP_ROWS_TETRIS; i++)
    for(j = 0; j < NUM_DISP_COLS_TETRIS; j++)
      bigDispBoard[i+1][j+6] = DISP_COLOR_GREEN;

  displayLEDTree(true, false);

  FastLED.show();
  delay(20);
}


void display_every_other()
{

  int numLightsBeforeRow = 0;

  for(int i = 0; i < NUM_DISP_ROWS_TREE; i++) // num rows
  {
    numLightsBeforeRow += ledsInRows[i]; // previous row
    for(int j = 0; j < ledsInRows[i + 1]; j++) // num lights current row
    {
      leds[NUM_LEDS + numLightsBeforeRow + j] = numToColorTree[i % 2 + 4];
    }
  }

  FastLED.show();
  delay(5);
}

void display_vortex(unsigned char Color1, unsigned char Color2)
{

  unsigned int led_num = NUM_LEDS;

  // (twinkle, twinkle + 19) are red
  // (twinkle + 20, twinkle + 39) are green

  for(int i = 0; i < NUM_TREE_LEDS; i++) // num rows
  {
    if ((((i + light_twinkle) % 40) / 20) == 0)
      leds[NUM_LEDS + i] = numToColorTree[Color1];
    else
      leds[NUM_LEDS + i] = numToColorTree[Color2];
  }
  
  light_twinkle = (light_twinkle + 1) % 40; // which column

  FastLED.show();
  delay(5);
}







void display_spiral(unsigned char Color1, unsigned char Color2, unsigned char Color3, unsigned char Color4)
{

    float tangentsByTen_1[18] =
      {-0.01, 0.17632,0.36397,0.57735,0.839099,1.19175,
       1.73205,2.74747,5.67128,10000000.0};

    float tangentsByTen_2[10] =
      {-10000000.0,-5.67128,
       -2.74747,-1.73205,-1.19175,-0.839099,-0.57735,
       -0.36397,-0.17632, 0.01};

    int sector = 0; // sector is from 0 - 35
    float tan_theta = 0;  // tan theta = col / row
    int x_coord = 0, y_coord = 0;

  // board is 22 x 22, place center at row 11, col 11
  for(int i = 0; i < NUM_DISP_ROWS; i++)
  {
    for(int j = 0; j < NUM_DISP_COLS; j++)
    {
      x_coord = j - 11;
      y_coord = i - 11;
      
      // if point is within (light_twinkle, light_twinkle + 90) degress, then it is Color1
      // i.e. within the next 9 10-degree sectors
      // First, identify sector of current point
      if (x_coord != 0)
         tan_theta = ((float)y_coord)/((float)x_coord);
      
      if ((x_coord > 0) && (y_coord >= 0)) // 1st quadrant
      {
         for(int k = 0; k < 9; k++)
         {
            if ((tan_theta >= tangentsByTen_1[k]) && (tan_theta < tangentsByTen_1[k + 1]))
               sector = k;
         }
      }
      else if ((x_coord < 0) && (y_coord >= 0)) // 2nd quadrant
      {
         for(int k = 0; k < 9; k++)
         {
            if ((tan_theta >= tangentsByTen_2[k]) && (tan_theta < tangentsByTen_2[k + 1]))
               sector = k + 9;
         }
      }
      else if ((x_coord < 0) && (y_coord < 0)) // 3rd quadrant
      {
         for(int k = 0; k < 9; k++)
         {
            if ((tan_theta >= tangentsByTen_1[k]) && (tan_theta < tangentsByTen_1[k + 1]))
               sector = k + 18;
         }
      }
      else if ((x_coord > 0) && (y_coord < 0)) // 4th quadrant
      {
         for(int k = 0; k < 9; k++)
         {
            if ((tan_theta >= tangentsByTen_2[k]) && (tan_theta < tangentsByTen_2[k + 1]))
               sector = k + 27;
         }
      }
      else if ((x_coord == 0) && (y_coord > 0)) // postive y axis
      {
        sector = 9;
      }
      else if ((x_coord == 0) && (y_coord < 0)) // negative y axis
      {
        sector = 27;
      }

      if (((sector + 36 - light_twinkle) % 36) < 9)
        bigDispBoard[i][j] = Color1;
      else if (((sector + 36 - light_twinkle) % 36) < 18)
        bigDispBoard[i][j] = Color2;
      else if (((sector + 36 - light_twinkle) % 36) < 27)
        bigDispBoard[i][j] = Color3;
      else 
        bigDispBoard[i][j] = Color4;
    }
  }
  
  light_twinkle = (light_twinkle + 1) % 36; // degrees by 10

  displayLEDTree(true, false);
  delay(50);
}


void display_clear()
{

  for(int i = 0; i < NUM_LEDS + NUM_TREE_LEDS; i++) 
  {
     leds[i] = CRGB::Black;
  }

  FastLED.show();
  delay(5);
}

void display_rows_move(unsigned char Color1, unsigned char Color2)
{
  unsigned int current_led = NUM_LEDS;
  for(int i = 0; i < NUM_DISP_ROWS_TREE; i++)
  {
    unsigned char cur_row_color = Color1;
    if ((((i + (light_twinkle / 2)) % 4) / 2) == 0)
      cur_row_color = Color2;

    for(int j = 0; j < ledsInRows[i]; j++)
    {
       leds[current_led] = numToColorTree[cur_row_color];
       current_led++;
    }

  }

  light_twinkle = (light_twinkle + 1) % 8; 

  FastLED.show();
  delay(40);
}





































/* Game stuff */
#define NORMAL_JUMP_TIME 8

bool game_is_mario = false;
bool hero_was_in_air = false;
bool hero_let_go_of_a = false; /* user let go of a button */
unsigned int hero_jump_count = 0; /* time when mario most recently started jumping */
unsigned char hero_jump_time_accel = NORMAL_JUMP_TIME;


/* MARIO Functions */


  /* Mario Left Standing Display */
const unsigned char PROGMEM marioDispOne[NUM_ROWS_MARIO_RUN][NUM_COLS_MARIO_RUN] =
{{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10}, // 0
 {10,10,10,10,10,04,04,04,04,04,04,10,10,10,10,10,10}, // 1
 {10,10,04,04,04,04,04,04,04,04,04,04,10,10,10,10,10}, // 2
 {10,10,10,10,07,01,07,07,13,13,13,13,10,10,10,10,10}, // 3
 {10,10,07,07,07,01,07,07,07,13,07,13,13,10,10,10,10}, // 4
 {10,07,07,07,13,07,07,07,13,13,07,13,13,10,10,10,10}, // 5
 {10,10,13,13,13,13,07,07,07,07,13,13,13,10,10,10,10}, // 6
 {10,10,10,07,07,07,07,07,07,07,07,10,10,10,10,10,10}, // 7
 {10,10,10,10,10,04,04,04,01,04,04,04,10,10,10,10,10}, // 8
 {10,10,04,04,04,01,04,04,01,04,04,04,04,10,10,10,10}, // 9
 {10,04,04,04,04,01,01,01,01,04,04,04,04,04,10,10,10}, // 10
 {10,11,11,04,01,03,01,01,03,01,04,11,11,11,10,10,10}, // 11
 {10,11,11,11,01,01,01,01,01,01,11,11,11,11,10,10,10}, // 12
 {10,11,11,01,01,01,01,01,01,01,01,11,11,11,10,10,10}, // 13
 {10,10,10,01,01,01,01,10,01,01,01,01,10,10,10,10,10}, // 14
 {10,10,13,13,13,13,10,10,10,13,13,13,13,10,10,10,10}, // 15
 {10,13,13,13,13,13,10,10,10,13,13,13,13,13,10,10,10}, // 16
 {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10}  // 17
 };

 /* Mario Left Running One Display */
const unsigned char PROGMEM marioDispTwo[NUM_ROWS_MARIO_RUN][NUM_COLS_MARIO_RUN] =
{{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10}, // 0
 {10,10,10,10,10,04,04,04,04,04,04,10,10,10,10,10,10}, // 1
 {10,10,04,04,04,04,04,04,04,04,04,04,10,10,10,10,10}, // 2
 {10,10,10,10,07,01,07,07,13,13,13,13,10,10,10,10,10}, // 3
 {10,10,07,07,07,01,07,07,07,13,07,13,13,10,10,10,10}, // 4
 {10,07,07,07,13,07,07,07,13,13,07,13,13,10,10,10,10}, // 5
 {10,10,13,13,13,13,07,07,07,07,13,13,13,10,10,10,10}, // 6
 {10,10,10,07,07,07,07,07,07,07,07,10,10,10,10,10,10}, // 7
 {10,10,10,11,11,04,01,04,04,04,04,04,10,10,10,10,10}, // 8
 {10,10,11,11,11,04,04,04,04,04,04,11,11,10,10,10,10}, // 9
 {10,10,10,11,11,04,04,04,04,04,01,11,11,11,10,10,10}, // 10
 {10,10,10,10,01,01,01,01,01,01,01,13,13,13,10,10,10}, // 11
 {10,10,10,10,01,01,01,01,01,01,01,01,13,13,10,10,10}, // 12
 {10,10,10,10,10,01,01,01,01,01,01,01,13,13,13,10,10}, // 13
 {10,10,10,10,10,10,13,13,13,13,10,10,10,13,13,10,10}, // 14
 {10,10,10,10,10,13,13,13,13,13,10,10,10,10,10,10,10}, // 15
 {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10}, // 16
 {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10}  // 17
 };

 /* Mario Left Running Two Display */
const unsigned char PROGMEM marioDispThree[NUM_ROWS_MARIO_RUN][NUM_COLS_MARIO_RUN] =
{{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10}, // 0
 {10,10,10,10,10,04,04,04,04,04,04,10,10,10,10,10,10}, // 1
 {10,10,04,04,04,04,04,04,04,04,04,04,10,10,10,10,10}, // 2
 {10,10,10,10,07,01,07,07,13,13,13,13,10,10,10,10,10}, // 3
 {10,10,07,07,07,01,07,07,07,13,07,13,13,10,10,10,10}, // 4
 {10,07,07,07,13,07,07,07,13,13,07,13,13,10,10,10,10}, // 5
 {10,10,13,13,13,13,07,07,07,07,13,13,13,10,10,10,10}, // 6
 {10,10,10,07,07,07,07,07,07,07,07,10,10,10,10,10,10}, // 7
 {10,10,10,10,10,04,04,04,01,04,04,04,10,10,10,10,10}, // 8
 {10,10,10,10,04,04,01,01,04,04,04,04,04,10,10,10,10}, // 9
 {10,10,10,03,01,01,03,01,01,04,04,04,04,10,10,10,10}, // 10
 {10,10,10,01,01,01,01,01,04,04,04,04,04,10,10,10,10}, // 11
 {10,10,10,01,01,01,11,11,11,04,04,01,01,10,10,10,10}, // 12
 {10,10,10,10,01,01,01,11,11,04,01,01,10,10,10,10,10}, // 13
 {10,10,10,10,13,13,13,01,01,01,01,10,10,10,10,10,10}, // 14
 {10,10,10,13,13,13,13,13,13,13,13,10,10,10,10,10,10}, // 15
 {10,10,10,10,10,10,13,13,13,13,13,10,10,10,10,10,10}, // 16
 {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10}  // 17
 };

 /* Mario Left Running Three Display */
const unsigned char PROGMEM marioDispFour[NUM_ROWS_MARIO_RUN][NUM_COLS_MARIO_RUN] =
{{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10}, // 0
 {10,10,10,10,10,04,04,04,04,04,04,10,10,10,10,10,10}, // 1
 {10,10,04,04,04,04,04,04,04,04,04,04,10,10,10,10,10}, // 2
 {10,10,10,10,07,01,07,07,13,13,13,13,10,10,10,10,10}, // 3
 {10,10,07,07,07,01,07,07,07,13,07,13,13,10,10,10,10}, // 4
 {10,07,07,07,13,07,07,07,13,13,07,13,13,10,10,10,10}, // 5
 {10,10,13,13,13,13,07,07,07,07,13,13,13,10,10,10,10}, // 6
 {10,10,10,07,07,07,07,07,07,07,07,10,10,10,10,10,10}, // 7
 {10,10,10,10,10,04,04,01,01,04,04,04,04,04,10,10,10}, // 8
 {11,11,11,04,04,04,01,01,01,04,04,04,04,11,11,11,10}, // 9
 {11,11,04,04,01,01,01,03,01,04,04,04,11,11,11,11,10}, // 10
 {10,13,13,10,01,01,01,01,01,01,01,01,10,11,11,11,10}, // 11
 {10,13,13,01,01,01,01,01,01,01,01,01,01,10,10,10,10}, // 12
 {10,13,13,01,01,01,01,01,01,01,01,01,01,01,10,10,10}, // 13
 {10,13,13,01,01,01,01,10,10,01,01,01,13,13,13,10,10}, // 14
 {10,10,10,10,10,10,10,10,10,10,10,13,13,13,13,10,10}, // 15
 {10,10,10,10,10,10,10,10,10,10,13,13,13,13,10,10,10}, // 16
 {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10}  // 17
 };

 /* Mario Left Skid Display */
const unsigned char PROGMEM marioDispFive[NUM_ROWS_MARIO_RUN][NUM_COLS_MARIO_RUN] =
{{10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10}, // 0
 {10,10,10,10,10,04,04,04,04,04,04,10,10,10,10,10,10}, // 1
 {10,10,10,04,04,04,04,04,04,04,04,13,13,10,10,10,10}, // 2
 {10,10,10,10,07,01,07,13,13,13,13,13,13,13,10,10,10}, // 3
 {10,10,07,07,07,07,07,07,13,07,07,13,11,11,11,10,10}, // 4
 {10,07,07,13,13,07,07,13,13,07,07,13,11,11,11,10,10}, // 5
 {10,10,13,13,07,07,07,07,07,07,13,11,11,11,10,10,10}, // 6
 {10,10,10,07,07,01,04,04,04,01,01,01,01,10,10,10,10}, // 7
 {10,10,04,04,04,01,01,04,11,11,11,01,01,01,10,10,10}, // 8
 {10,10,04,04,04,04,04,04,11,11,11,04,01,01,10,10,10}, // 9
 {10,10,04,04,04,04,04,04,11,11,01,01,01,01,10,10,10}, // 10
 {10,10,10,04,04,04,04,01,01,01,01,01,01,10,10,10,10}, // 11
 {10,10,10,10,01,01,01,01,13,13,13,01,01,10,10,10,10}, // 12
 {10,10,10,10,01,01,01,13,13,13,13,13,10,10,10,10,10}, // 13
 {10,10,10,10,10,01,13,13,13,01,01,13,13,13,13,10,10}, // 14
 {10,10,10,10,10,10,10,10,01,13,13,13,13,13,13,10,10}, // 15
 {10,10,10,10,10,10,10,10,10,13,13,13,13,13,10,10,10}, // 16
 {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10}  // 17
 };

 /* Mario Left Jump Display */
const unsigned char PROGMEM marioDispSix[NUM_ROWS_MARIO_RUN][NUM_COLS_MARIO_RUN] =
{{11,11,11,10,10,10,10,10,10,10,10,10,10,10,10,10,10}, // 0
 {11,11,11,10,10,04,04,04,04,04,04,10,10,10,10,10,10}, // 1
 {11,11,04,04,04,04,04,04,04,04,04,04,10,10,10,10,10}, // 2
 {04,04,04,04,07,01,07,07,13,13,13,13,10,10,10,10,10}, // 3
 {04,04,07,07,07,01,07,07,07,13,07,13,13,10,10,10,10}, // 4
 {04,07,07,07,13,07,07,07,13,13,07,13,13,10,10,10,10}, // 5
 {10,04,13,13,13,13,07,07,07,07,13,13,13,10,10,10,10}, // 6
 {10,10,04,07,07,07,07,07,07,07,07,10,10,10,10,10,10}, // 7
 {10,10,10,04,01,04,04,04,01,04,04,04,04,04,04,10,10}, // 8
 {13,13,10,01,04,04,04,01,04,04,04,04,04,04,04,04,10}, // 9
 {13,13,10,01,01,01,01,01,04,04,04,04,04,04,11,11,11}, // 10
 {13,13,01,03,01,01,03,01,01,04,01,01,01,11,11,11,11}, // 11
 {13,13,01,01,01,01,01,01,01,01,01,01,13,13,11,11,10}, // 12
 {13,13,01,01,01,01,01,01,01,01,01,13,13,13,13,10,10}, // 13
 {10,10,10,10,10,01,01,01,01,01,01,01,13,13,13,13,10}, // 14
 {10,10,10,10,10,10,10,10,01,01,01,01,01,10,13,13,10}, // 15
 {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10}, // 16
 {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10}  // 17
 };

  /* Peach Left Standing Display */
const unsigned char PROGMEM peachDispOne[NUM_ROWS_MARIO_RUN][NUM_COLS_MARIO_RUN] =
{{10,10,10,10,10,10,27,10,27,10,27,10,10,10,10,10,10}, // 0
 {10,10,10,10,10,10,01,27,19,27,01,10,10,10,10,10,10}, // 1
 {10,10,10,10,10,10,03,03,03,03,03,03,10,10,10,10,10}, // 2
 {10,10,10,10,10,03,03,03,03,03,03,03,03,10,10,10,10}, // 3
 {10,10,10,10,03,03,07,03,03,03,03,03,03,10,10,10,10}, // 4 
 {10,10,10,10,03,03,07,01,07,03,03,03,03,03,03,10,10}, // 5
 {10,10,10,10,10,07,07,10,07,03,07,03,03,03,10,10,10}, // 6 g
 {10,10,10,10,10,07,07,07,07,03,01,03,03,10,10,10,10}, // 7 g
 {10,10,10,10,10,10,07,07,07,07,03,03,03,03,10,10,10}, // 8 g
 {10,10,10,10,10,26,01,03,19,19,19,03,03,03,03,10,10}, // 9 g
 {10,10,10,10,26,26,26,26,26,26,26,26,03,03,10,10,10}, // 10 g
 {10,10,10,10,07,26,26,26,26,26,07,07,03,10,10,10,10}, // 11 g
 {10,10,10,11,11,19,19,19,19,19,19,11,11,10,10,10,10}, // 12 g
 {10,10,11,11,19,19,26,26,19,19,19,19,11,11,10,10,10}, // 13 g
 {10,10,10,10,19,26,26,26,26,26,19,19,10,10,10,10,10}, // 14 g
 {10,10,10,10,26,26,26,26,26,26,26,26,10,10,10,10,10}, // 15 g
 {10,10,10,26,26,26,26,26,26,26,26,26,26,10,10,10,10}, // 16 g
 {10,10,10,19,19,19,19,19,19,19,19,19,19,19,10,10,10}  // 17
 };

 /* Daisy Left Standing Display */
const unsigned char PROGMEM daisyDispOne[NUM_ROWS_MARIO_RUN][NUM_COLS_MARIO_RUN] =
{{10,10,10,10,10,10,27,10,27,10,27,10,10,10,10,10,10}, // 0
 {10,10,10,10,10,10,19,27,22,27,19,10,10,10,10,10,10}, // 1
 {10,10,10,10,10,10,13,13,13,13,13,13,10,10,10,10,10}, // 2
 {10,10,10,10,10,13,13,13,13,13,13,13,13,10,10,10,10}, // 3
 {10,10,10,10,13,13,07,13,13,13,13,13,13,10,10,10,10}, // 4
 {10,10,10,10,13,13,07,01,07,13,13,13,13,13,13,10,10}, // 5
 {10,10,10,10,10,07,07,10,07,13,11,13,13,13,10,10,10}, // 6 g
 {10,10,10,10,10,07,07,07,07,11,22,11,13,10,10,10,10}, // 7 g
 {10,10,10,10,10,10,07,07,07,07,11,13,13,13,10,10,10}, // 8 g
 {10,10,10,10,10,03,11,11,11,11,03,13,13,13,13,10,10}, // 9 9
 {10,10,10,10,03,03,22,11,03,03,03,03,13,13,10,10,10}, // 10 g
 {10,10,10,10,07,03,03,03,03,03,07,07,13,10,10,10,10}, // 11 g
 {10,10,10,11,11,02,02,02,02,02,02,11,11,10,10,10,10}, // 12 g
 {10,10,11,11,02,02,03,03,02,02,02,02,11,11,10,10,10}, // 13 g
 {10,10,10,10,02,03,03,03,03,03,02,02,10,10,10,10,10}, // 14 g
 {10,10,10,10,03,03,03,03,03,03,03,03,10,10,10,10,10}, // 15 g
 {10,10,10,03,03,03,03,03,03,03,03,03,03,10,10,10,10}, // 16 g
 {10,10,10,02,02,02,02,02,02,02,02,02,02,02,10,10,10}  // 17
 };

#define PLAY_MARIO_RED   1
#define PLAY_MARIO_GREEN 2
#define PLAY_MARIO_PINK 3
#define PLAY_MARIO_ORANGE 4


unsigned char disp_mario_luigi = 0; // 0 - Mario, 1 - Luigi, 2 - Peach, 3 - Daisy
unsigned char num_hero_jumps = 0;







/* GIMKIT */

#define NUM_GIMKIT_ROWS 256
#define NUM_GIMKIT_COLS 256

#define NUM_GIMKIT_PLATFORMS 16

#define GIMKIT_HERO_WIDTH 2
#define GIMKIT_HERO_HEIGHT 3

unsigned char gimkit_current_platform = 0;


/* Gimkit Platform Row */
/* 0 is topmost row, 255 is bottom */
const unsigned char PROGMEM gkPlatform_Row[NUM_GIMKIT_PLATFORMS] =
/* 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 */
{255,245,243,240,236,234,234,230,225,220,220,215,215,215,215,208}; // 1

/* Gimkit Platform Col */
/* 0 is leftmost col, 255 is rightmost */
const unsigned char PROGMEM gkPlatform_Col[NUM_GIMKIT_PLATFORMS] =
/* 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 */
{  0, 12, 19, 26, 32, 36, 40, 52, 58, 68, 74, 78, 88, 94,100,106}; // 1

/* Gimkit Platform Length */
/* Platform Length */
const unsigned char PROGMEM gkPlatform_Length[NUM_GIMKIT_PLATFORMS] =
/* 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 */
{255,  4,  4,  3,  2,  2,  8,  2,  6,  2,  1,  6,  2,  2,  1, 10}; // 1










/* Only called in 22x22 mode at the moment */
/* Frames numbered 1-6 */
/* disp_mario_luigi: 0 - Mario, 1 - Luigi, 2 - Peach, 3 - Daisy */
void displayLEDBoardMarioRun(unsigned char mario_frame, unsigned char mario_dir)
{
  unsigned int i, j;
  unsigned int unused_cols_left = 2;
  unsigned int unused_rows_top = 1;
  unsigned int color_index = 0;
  unsigned int column_idx = 0;
  
    
  for(i = 0; i < NUM_DISP_ROWS; i++)
  {
    for(j = 0; j < NUM_DISP_COLS; j++)
    {
      bigDispBoard[i][j] = DISP_COLOR_LIGHT_BLUE;
    }
  }
  
  /* bottom row to top, right to left */
  for(i = 0; i < NUM_ROWS_MARIO_RUN; i++)
  {
    for(j = 0; j < NUM_COLS_MARIO_RUN; j++)
    {      

      if (mario_dir == MOVE_LEFT)
        column_idx = j;
      else
        column_idx = NUM_COLS_MARIO_RUN - j - 1;

      if (disp_mario_luigi == 2) // peach
        color_index = pgm_read_byte_near(&peachDispOne[i][column_idx]);
      else if (disp_mario_luigi == 3) // daisy
        color_index = pgm_read_byte_near(&daisyDispOne[i][column_idx]);
      else if (mario_frame == 1)
        color_index = pgm_read_byte_near(&marioDispOne[i][column_idx]);
      else if (mario_frame == 2)
        color_index = pgm_read_byte_near(&marioDispTwo[i][column_idx]);
      else if (mario_frame == 3)
        color_index = pgm_read_byte_near(&marioDispThree[i][column_idx]);
      else if (mario_frame == 4)
        color_index = pgm_read_byte_near(&marioDispFour[i][column_idx]);
      else if (mario_frame == 5)
        color_index = pgm_read_byte_near(&marioDispFive[i][column_idx]);
      else if (mario_frame == 6)
        color_index = pgm_read_byte_near(&marioDispSix[i][column_idx]);
      
      if ((disp_mario_luigi == 1) and (color_index == DISP_COLOR_RED))
        color_index = DISP_COLOR_GREEN;

      

      bigDispBoard[i + unused_rows_top][j + unused_cols_left] = color_index;
    }
  }

  displayLEDs(true, false);
}

#define MARIO_DELAY_TIME 100

unsigned char run_move_dir = MOVE_NONE;
unsigned char mario_dir = MOVE_RIGHT;
boolean mario_move = false; // if moving this iteration
unsigned char mario_frame = 1; // 1-6: 1-Stand, 2-Run1, 3-Run2, 4-Run3, 5-Skid, 6-Jump
unsigned char mario_run_timer = 0; // TODO: Provide description
unsigned int delay_time = 100;
void display_mario_run()
{
  unsigned int i, j;
  unsigned int move_dir = MOVE_NONE;
  unsigned int button_press;
  bool mario_over = false;
  
  while (mario_over == false)
  {
    delay(delay_time - 20);
    
    delay_time = MARIO_DELAY_TIME;
    if (mario_run_timer > 0)
      mario_run_timer--;

    /* look for movements */
    move_dir = getMove();
    if (move_dir == MOVE_SELECT)
    {
      disp_mario_luigi = (disp_mario_luigi + 1) % 4;
    }
    else if (move_dir == MOVE_START)
    {
      mario_over = true;
    }
    else if (((move_dir & 0xF) == MOVE_RIGHT) || ((move_dir & 0xF) == MOVE_LEFT))
    {
      if ((mario_run_timer > 0) && ((run_move_dir & 0xF) != (move_dir & 0xF)))
      {
        mario_frame = 5;
        delay_time = delay_time * 3;
      }
      else if ((mario_move == false) || (mario_frame == 6))
        mario_frame = 2;
      else
        mario_frame = (mario_frame + 2) % 3 + 2;
      mario_dir = move_dir & 0xF;
      mario_move = true;
    }
    else
    {
      mario_move = false;
      mario_frame = 1; // Standing
    }

    button_press = move_dir >> 4;
    if (button_press == MOVE_ROTATE_RIGHT) // A Button, Jump
    {
      delay_time = delay_time * 4;
      mario_frame = 6;
      
    }
    else if (button_press == MOVE_ROTATE_LEFT) // B Button, Run
    {
      delay_time = delay_time / 2;
      mario_run_timer = 2;
      run_move_dir = move_dir & 0xF;
    }

    displayLEDBoardMarioRun(mario_frame, mario_dir);
    delay(10);
  }

}











/* MARIO GAME */

#define MARIO_FACE_COLOR DISP_COLOR_PEACH
#define MARIO_HAT_COLOR DISP_COLOR_RED
#define LUIGI_HAT_COLOR DISP_COLOR_GREEN
#define MARIO_PANTS_COLOR DISP_COLOR_BLUE
#define MARIO_SHOE_COLOR DISP_COLOR_BROWN
#define MARIO_FIRE_HAT_COLOR DISP_COLOR_WHITE
#define MARIO_FIRE_PANTS_COLOR DISP_COLOR_RED

#define HERO_ACCELERATION 0.2
#define NUM_MARIO_COLUMNS 448
#define MARIO_INVUL_COUNT 60

bool hero_face_right = true;
bool mario_is_big = false;
bool mario_is_fire = false;
bool hero_is_trying = false;
bool mario_is_star = false;
unsigned int game_count = 0; /* count of loop cycles for current game */

unsigned int mario_multi_brick_count = 0; /* count used for multi-brick */
unsigned int mario_bump_brick_count = 0; /* count used for bump brick */
unsigned int mario_invulnerable_count = 0; /* count used for when mario invulnerable for a bit */

unsigned int total_score = 0;
        
#define MARIO_LOW_PIPE_TOP        0x1     /* top of small pipe */
#define MARIO_MED_PIPE_TOP        0x2     /* top of large pipe */
#define MARIO_HIGH_PIPE_TOP       0x4     /* base of pipe, from pipe top to ground */
#define MARIO_HOLE                0x8     /* hole in ground in which Mario may fall */
#define MARIO_LOW_BRICK           0x10    /* lower level brick */
#define MARIO_HIGH_BRICK          0x20    /* higher level brick */
#define MARIO_LOW_Q               0x40    /* lower level ? block */
#define MARIO_HIGH_Q              0x80    /* higher level ? block */
#define MARIO_STEP_1              0x100   /* 1 pixel height step */
#define MARIO_STEP_2              0x200   /* 2 pixel height step */
#define MARIO_STEP_3              0x400   /* 3 pixel height step */
#define MARIO_STEP_4              0x800   /* 4 pixel height step */
#define MARIO_STEP_5              0x1000  /* 5 pixel height step */
#define MARIO_STEP_6              0x2000  /* 6 pixel height step */
#define MARIO_STEP_7              0x4000  /* 7 pixel height step */
#define MARIO_STEP_8              0x8000  /* 8 pixel height step */

 /* Mario Foreground Display Items */
const unsigned int PROGMEM marioDispForeItems[NUM_MARIO_COLUMNS] =
/*    0,     1,     2,     3,     4,     5,     6,     7,     8,     9,    10,    11,    12,    13,    14,    15 */
{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, // 1
 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, // 2
 
 0x0000,0x0000,0x0040,0x0040,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0010,0x0010,0x0040,0x0040,0x0090,0x0090, // 3
 0x0040,0x0040,0x0010,0x0010,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0001,0x0001,0x0001,0x0001,0x0000,0x0000, // 4
 
 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0002,0x0002, // 5
 0x0002,0x0002,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0004,0x0004, // 6
 
 0x0004,0x0004,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, // 7
 0x0000,0x0000,0x0000,0x0000,0x0004,0x0004,0x0004,0x0004,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, // 8
 
 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0008,0x0008,0x0008,0x0008, // 9
 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, // 10
 
 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0010,0x0010,0x0040,0x0040,0x0010,0x0010,0x0020,0x0020, // 11
 0x0020,0x0020,0x0020,0x0020,0x0020,0x0020,0x0020,0x0020,0x0020,0x0020,0x0028,0x0028,0x0028,0x0028,0x0008,0x0008, // 12
 
 0x0000,0x0000,0x0020,0x0020,0x0020,0x0020,0x0020,0x0020,0x0020,0x0020,0x0090,0x0090,0x0000,0x0000,0x0000,0x0000, // 13
 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0010,0x0010,0x0010,0x0010,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, // 14
 
 0x0000,0x0000,0x0040,0x0040,0x0000,0x0000,0x0000,0x0000,0x00C0,0x00C0,0x0000,0x0000,0x0000,0x0000,0x0040,0x0040, // 15
 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0010,0x0010,0x0000,0x0000,0x0000,0x0000, // 16
 
 0x0000,0x0000,0x0020,0x0020,0x0020,0x0020,0x0020,0x0020,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, // 17
 0x0020,0x0020,0x0090,0x0090,0x0090,0x0090,0x0020,0x0020,0x0000,0x0000,0x0000,0x0000,0x0100,0x0100,0x0300,0x0300, // 18
 
 0x0700,0x0700,0x0F00,0x0F00,0x0000,0x0000,0x0000,0x0000,0x0F00,0x0F00,0x0700,0x0700,0x0300,0x0300,0x0100,0x0100, // 19
 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0100,0x0100,0x0300,0x0300,0x0700,0x0700,0x0F00,0x0F00, // 20
 
 0x0F00,0x0F00,0x0008,0x0008,0x0008,0x0008,0x0F00,0x0F00,0x0700,0x0700,0x0300,0x0300,0x0100,0x0100,0x0000,0x0000, // 21
 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0001,0x0001,0x0001,0x0001,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, // 22
 
 0x0000,0x0000,0x0010,0x0010,0x0010,0x0010,0x0040,0x0040,0x0010,0x0010,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, // 23
 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0001,0x0001,0x0001,0x0001,0x0100,0x0100,0x0300,0x0300, // 24
 
 0x0700,0x0700,0x0F00,0x0F00,0x1F00,0x1F00,0x3F00,0x3F00,0x7F00,0x7F00,0xFF00,0xFF00,0xFF00,0xFF00,0x0000,0x0000, // 25
 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0100,0x0100,0x0000,0x0000, // 26
 
 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, // 27
 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000};// 28


#define NUM_BRICKS 20
#define NUM_Q 13
#define NUM_HIGH_Q 10

#define MARIO_1UP_COL 130
#define MARIO_REPEAT_BRICK_COL 202
#define MARIO_STAR_COL 216

#define MARIO_RESTART_COL 186

/* These are per column */
/* These represent current locations of bricks (haven't been hit), populated from marioDispForeItems */
unsigned int locations_high_bricks[NUM_BRICKS] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned int locations_low_bricks[NUM_BRICKS] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
/* These are per Q */
/* These represent if a Q has been hit */
unsigned int locations_low_q[NUM_Q] = {0,0,0,0,0,0,0,0,0,0,0,0}; /* Added 1 for 1UP, 1 for repeat brick, 1 for star */
unsigned int locations_high_q[NUM_HIGH_Q] = {0,0,0,0,0,0,0,0,0,0};

unsigned char high_q_i = 0;
unsigned char low_q_i = 0;

unsigned const int low_mush_fire[2] = {44, 170};
unsigned const int high_mush_fire = 232;

unsigned char mush_row = 0;
unsigned int mush_col = 0;
bool mush_is_red = true;
bool mush_go_right = true;
unsigned int mush_count = 0;
unsigned char mario_lives = 2;
bool mario_1up_hit = false;
bool mush_is_flower = false;
bool mario_star_hit = false;


#define MARIO_DISP_CLOUD_1       0x1     /* low height cloud */
#define MARIO_DISP_CLOUD_2       0x2     /* medium height cloud */
#define MARIO_DISP_BUSH_1        0x4     /* 1 pixel height bush */
#define MARIO_DISP_BUSH_2        0x8     /* 2 pixel height bush */
#define MARIO_DISP_HILL_1        0x10    /* 1 pixel height hill */
#define MARIO_DISP_HILL_2        0x20    /* 2 pixel height hill */
#define MARIO_DISP_HILL_3        0x40    /* 3 pixel height hill */
#define MARIO_DISP_HILL_4        0x80    /* 2 pixel height hill */
#define MARIO_DISP_HILL_5        0x100   /* 3 pixel height hill */
#define MARIO_CASTLE_LOW_WALL    0x200   /* 6 pixel height lower wall */
#define MARIO_CASTLE_DOOR        0x400   /* 4 pixel height lower wall door */
#define MARIO_CASTLE_LOW_TURRET  0x800   /* 1 pixel height lower turret */
#define MARIO_CASTLE_HIGH_WALL   0x1000  /* 6 pixel height lower wall */
#define MARIO_CASTLE_WINDOW      0x2000   /* 4 pixel height lower wall door */
#define MARIO_CASTLE_HIGH_TURRET 0x4000   /* 1 pixel height lower turret */
#define MARIO_DISP_CLOUD_3       0x8000   /* high height cloud */

#define MARIO_FLAG_POLE_COL      413

 /* Mario Background Display Items */
const unsigned int PROGMEM marioDispBackItems[NUM_MARIO_COLUMNS] =
/*    0,     1,     2,     3,     4,     5,     6,     7,     8,     9,    10,    11,    12,    13,    14,    15 */
{0x0010,0x0030,0x0070,0x00F0,0x01F0,0x01F0,0x00F0,0x0070,0x0030,0x0010,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, // 1
 0x0000,0x0000,0x0001,0x0003,0x0003,0x0001,0x0004,0x000C,0x000C,0x0004,0x000C,0x000C,0x0004,0x000C,0x000C,0x0004, // 2
 
 0x0000,0x0000,0x0010,0x0030,0x0070,0x0070,0x0030,0x0010,0x0002,0x8002,0x8002,0x0002,0x0000,0x0000,0x0000,0x0000, // 3
 0x0000,0x0000,0x0004,0x000C,0x000C,0x0004,0x0001,0x0003,0x0003,0x0001,0x0003,0x0003,0x0001,0x0003,0x0003,0x0001, // 4
 
 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0002,0x8002,0x8002,0x0002, // 5
 0x8002,0x8002,0x0002,0x0000,0x0000,0x0004,0x000C,0x000C,0x0004,0x000C,0x000C,0x0004,0x0000,0x0000,0x0000,0x0000, // 6
 
 0x0000,0x0000,0x0010,0x0030,0x0070,0x00F0,0x01F0,0x01F0,0x00F0,0x0070,0x0030,0x0010,0x0000,0x0000,0x0000,0x0000, // 7
 0x0000,0x0000,0x0000,0x0000,0x0001,0x0003,0x0003,0x0001,0x0004,0x000C,0x000C,0x0004,0x000C,0x000C,0x0004,0x000C, // 8
 
 0x000C,0x0004,0x0010,0x0030,0x0070,0x0070,0x0030,0x0010,0x0000,0x0000,0x0002,0x8002,0x8002,0x0002,0x0000,0x0000, // 9
 0x0000,0x0000,0x0004,0x000C,0x000C,0x0004,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, // 10
 
 0x0000,0x0000,0x0000,0x0001,0x0003,0x0003,0x0001,0x0003,0x0003,0x0001,0x0003,0x0003,0x0001,0x0000,0x0000,0x0000, // 11
 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0002,0x8002,0x8002,0x0002,0x8002,0x8002,0x0002,0x0000, // 12
 
 0x0000,0x0000,0x0004,0x000C,0x000C,0x0004,0x000C,0x000C,0x0004,0x0000,0x0000,0x0000,0x0000,0x0000,0x0010,0x0030, // 13
 0x0070,0x00F0,0x01F0,0x01F0,0x00F0,0x0070,0x0030,0x0010,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, // 14
 
 0x0001,0x0003,0x0003,0x0001,0x0004,0x000C,0x000C,0x0004,0x000C,0x000C,0x0004,0x000C,0x000C,0x0004,0x0010,0x0030, // 15
 0x0070,0x0070,0x0030,0x0010,0x0000,0x0000,0x0000,0x0000,0x0002,0x8002,0x8002,0x0002,0x0000,0x0000,0x0000,0x0000, // 16
 
 0x0004,0x000C,0x000C,0x0004,0x0000,0x0000,0x0001,0x0003,0x0003,0x0001,0x0003,0x0003,0x0001,0x0003,0x0003,0x0001, // 17
 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0002,0x8002,0x8002,0x0002,0x8002,0x8002,0x0002, // 18
 
 0x0000,0x0000,0x0000,0x0000,0x0004,0x000C,0x000C,0x0004,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, // 19
 0x0010,0x0030,0x0070,0x00F0,0x01F0,0x01F0,0x00F0,0x0070,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, // 20
 
 0x0000,0x0000,0x0001,0x0003,0x0003,0x0001,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0004,0x0000, // 21
 0x0010,0x0030,0x0070,0x0070,0x0030,0x0010,0x0000,0x0000,0x0000,0x0000,0x0002,0x8002,0x8002,0x0002,0x0000,0x0000, // 22
 
 0x0000,0x0000,0x0004,0x000C,0x000C,0x0004,0x0000,0x0000,0x0000,0x0001,0x0003,0x0003,0x0001,0x0003,0x0003,0x0001, // 23
 0x0003,0x0003,0x0001,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0002,0x8002,0x8002,0x0002, // 24
 
 0x8002,0x8002,0x0002,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0010,0x0030, // 25
 0x0070,0x00F0,0x01F0,0x01F0,0x00F0,0x0070,0x0030,0x0010,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, // 26
 
 0x0000,0x0000,0x0001,0x0003,0x0003,0x0001,0x0A00,0x0200,0x5A00,0x3200,0x5E00,0x5E00,0x3200,0x5A00,0x0200,0x0A00, // 27
 0x0004,0x0000,0x0010,0x0030,0x0070,0x0070,0x0030,0x0010,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000};// 28



void disp_mario_back(int current_display_col, unsigned char mario_color)
{
  unsigned char i,j;
  unsigned char mario_lives_color = DISP_COLOR_RED;
  if (mario_color == PLAY_MARIO_RED)
    mario_lives_color = DISP_COLOR_RED;
  else if (mario_color == PLAY_MARIO_GREEN)
    mario_lives_color = DISP_COLOR_GREEN;
  else if (mario_color == PLAY_MARIO_PINK)
    mario_lives_color = DISP_COLOR_PINK;
  else if (mario_color == PLAY_MARIO_ORANGE)
    mario_lives_color = DISP_COLOR_ORANGE;
  
  for(i = 0; i < NUM_DISP_ROWS; i++)
  {
    for(j = 0; j < NUM_DISP_COLS; j++)
    {
      if (i < NUM_DISP_ROWS - 1)
        bigDispBoard[i][j] = DISP_COLOR_LIGHT_BLUE;
      else if ((pgm_read_word_near(&marioDispForeItems[current_display_col+j]) & MARIO_HOLE) == 0) /* not a hole */
        bigDispBoard[i][j] = DISP_COLOR_HALF_RED;
      else
        bigDispBoard[i][j] = DISP_COLOR_BLACK;
    }
  }

  if (mario_lives > 0)
    bigDispBoard[0][0] = mario_lives_color;
  if (mario_lives > 1)
    bigDispBoard[0][1] = mario_lives_color;
  if (mario_lives > 2)
    bigDispBoard[0][2] = mario_lives_color;
  
  
}


void display_mario_back_items(int current_display_col)
{
  unsigned char i, j;
  for(j = 0; j < NUM_DISP_COLS; j++)
  {
    unsigned int back_item_word = pgm_read_word_near(&marioDispBackItems[current_display_col+j]);
    
    /* Display Clouds */
    if ((back_item_word & MARIO_DISP_CLOUD_1) > 0)
      bigDispBoard[3][j] = DISP_COLOR_HALF_WHITE;
    if ((back_item_word & MARIO_DISP_CLOUD_2) > 0)
      bigDispBoard[2][j] = DISP_COLOR_HALF_WHITE;
    if ((back_item_word & MARIO_DISP_CLOUD_3) > 0)
      bigDispBoard[1][j] = DISP_COLOR_HALF_WHITE;

    /* Display Bushes */
    if ((back_item_word & MARIO_DISP_BUSH_1) > 0)
      bigDispBoard[20][j] = DISP_COLOR_BUSH_GREEN;
    if ((back_item_word & MARIO_DISP_BUSH_2) > 0)
      bigDispBoard[19][j] = DISP_COLOR_BUSH_GREEN;

    /* Display Hills */
    if ((back_item_word & MARIO_DISP_HILL_1) > 0)
      bigDispBoard[20][j] = DISP_COLOR_HILL_GREEN;
    if ((back_item_word & MARIO_DISP_HILL_2) > 0)
      bigDispBoard[19][j] = DISP_COLOR_HILL_GREEN;
    if ((back_item_word & MARIO_DISP_HILL_3) > 0)
      bigDispBoard[18][j] = DISP_COLOR_HILL_GREEN;
    if ((back_item_word & MARIO_DISP_HILL_4) > 0)
      bigDispBoard[17][j] = DISP_COLOR_HILL_GREEN;
    if ((back_item_word & MARIO_DISP_HILL_5) > 0)
      bigDispBoard[16][j] = DISP_COLOR_HILL_GREEN;

    /* Display Castle */
    if ((back_item_word & MARIO_CASTLE_LOW_WALL) > 0)
      for(i = 15; i <= 20; i++)
        bigDispBoard[i][j] = DISP_COLOR_HALF_RED;
    if ((back_item_word & MARIO_CASTLE_HIGH_WALL) > 0)
      for(i = 9; i <= 14; i++)
        bigDispBoard[i][j] = DISP_COLOR_HALF_RED;
    if ((back_item_word & MARIO_CASTLE_DOOR) > 0)
      for(i = 17; i <= 20; i++)
        bigDispBoard[i][j] = DISP_COLOR_BLACK;
    if ((back_item_word & MARIO_CASTLE_WINDOW) > 0)
      for(i = 10; i <= 12; i++)
        bigDispBoard[i][j] = DISP_COLOR_BLACK;
    if ((back_item_word & MARIO_CASTLE_LOW_TURRET) > 0)
      bigDispBoard[14][j] = DISP_COLOR_YELLOW;
    if ((back_item_word & MARIO_CASTLE_HIGH_TURRET) > 0)
      bigDispBoard[8][j] = DISP_COLOR_YELLOW;

    /* Display Flag Pole */
    if ((current_display_col + j) == MARIO_FLAG_POLE_COL)
    {
      for(i = 2; i <= 19; i++)
        bigDispBoard[i][j] = DISP_COLOR_PIPE_GREEN;
      bigDispBoard[1][j] = DISP_COLOR_GREEN;
    }
  }
}







void display_mario_fore_items(int current_display_col)
{
  unsigned int i, j;
  unsigned char pipe_top = 0;

  for(j = 0; j < NUM_DISP_COLS; j++)
  {
    /* Display Pipes */
    pipe_top = 0;
    unsigned int fore_item_word = pgm_read_word_near(&marioDispForeItems[current_display_col+j]);
    if ((fore_item_word & MARIO_LOW_PIPE_TOP) > 0)
      pipe_top = 16;
    if ((fore_item_word & MARIO_MED_PIPE_TOP) > 0)
      pipe_top = 14;
    if ((fore_item_word & MARIO_HIGH_PIPE_TOP) > 0)
      pipe_top = 12;
    if (pipe_top > 0)
    {
      bigDispBoard[pipe_top][j] = DISP_COLOR_PIPE_GREEN;
      bigDispBoard[pipe_top + 1][j] = DISP_COLOR_PIPE_GREEN;

      /* Middle of pipe, draw pipe base */
      if (((pgm_read_word_near(&marioDispForeItems[current_display_col+j+1]) & (MARIO_HIGH_PIPE_TOP | MARIO_MED_PIPE_TOP | MARIO_LOW_PIPE_TOP)) > 0) &&
          ((pgm_read_word_near(&marioDispForeItems[current_display_col+j-1]) & (MARIO_HIGH_PIPE_TOP | MARIO_MED_PIPE_TOP | MARIO_LOW_PIPE_TOP)) > 0))
        for(i = pipe_top + 2; i <= 20; i++)
          bigDispBoard[i][j] = DISP_COLOR_PIPE_GREEN;
    }

    /* Display Bricks */
    for(i = 0; i < NUM_BRICKS; i++)
    {
      if ((locations_low_bricks[i] > 0) && (locations_low_bricks[i] == current_display_col + j))
      {
        bigDispBoard[13][j] = DISP_COLOR_HALF_RED;
        bigDispBoard[14][j] = DISP_COLOR_HALF_RED;
        if ((j + 1) < 22)
        {
          bigDispBoard[13][j + 1] = DISP_COLOR_HALF_RED;
          bigDispBoard[14][j + 1] = DISP_COLOR_HALF_RED;
        }
      }
      if ((locations_high_bricks[i] > 0) && (locations_high_bricks[i] == current_display_col + j))
      {
        bigDispBoard[5][j] = DISP_COLOR_HALF_RED;
        bigDispBoard[6][j] = DISP_COLOR_HALF_RED;
        if ((j + 1) < 22)
        {
          bigDispBoard[5][j + 1] = DISP_COLOR_HALF_RED;
          bigDispBoard[6][j + 1] = DISP_COLOR_HALF_RED;
        }
      }
    }

    /* Display ? Blocks */
    if ((fore_item_word & MARIO_LOW_Q) > 0)
    {
      bigDispBoard[13][j] = DISP_COLOR_Q_YELLOW;
      bigDispBoard[14][j] = DISP_COLOR_Q_YELLOW;
    }
    if ((fore_item_word & MARIO_HIGH_Q) > 0)
    {
      bigDispBoard[5][j] = DISP_COLOR_Q_YELLOW;
      bigDispBoard[6][j] = DISP_COLOR_Q_YELLOW;
    }

    /* Display ? Blocks */
    for(i = 0; i < NUM_Q; i++)
    {
      /* Q this column, here Q in left column represent whole block */
      if ((locations_low_q[i] > 0) && ((locations_low_q[i] == current_display_col + j) || (locations_low_q[i] == current_display_col + j - 1)))
      {
        bigDispBoard[13][j] = DISP_COLOR_Q_ORANGE;
        bigDispBoard[14][j] = DISP_COLOR_Q_ORANGE;
      }
      /* Q this column, here Q in left column represent whole block */
      if ((i < NUM_HIGH_Q) && (locations_high_q[i] > 0) && ((locations_high_q[i] == current_display_col + j) || (locations_high_q[i] == current_display_col + j - 1)))
      {
        bigDispBoard[5][j] = DISP_COLOR_Q_ORANGE;
        bigDispBoard[6][j] = DISP_COLOR_Q_ORANGE;
      }
    }

    /* Display Steps */
    if ((fore_item_word & MARIO_STEP_1) > 0)
    {
      bigDispBoard[19][j] = DISP_COLOR_RED;
      bigDispBoard[20][j] = DISP_COLOR_BROWN;
    }
    if ((fore_item_word & MARIO_STEP_2) > 0)
    {
      bigDispBoard[17][j] = DISP_COLOR_RED;
      bigDispBoard[18][j] = DISP_COLOR_BROWN;
    }
    if ((fore_item_word & MARIO_STEP_3) > 0)
    {
      bigDispBoard[15][j] = DISP_COLOR_RED;
      bigDispBoard[16][j] = DISP_COLOR_BROWN;
    }
    if ((fore_item_word & MARIO_STEP_4) > 0)
    {
      bigDispBoard[13][j] = DISP_COLOR_RED;
      bigDispBoard[14][j] = DISP_COLOR_BROWN;
    }
    if ((fore_item_word & MARIO_STEP_5) > 0)
    {
      bigDispBoard[11][j] = DISP_COLOR_RED;
      bigDispBoard[12][j] = DISP_COLOR_BROWN;
    }
    if ((fore_item_word & MARIO_STEP_6) > 0)
    {
      bigDispBoard[9][j] = DISP_COLOR_RED;
      bigDispBoard[10][j] = DISP_COLOR_BROWN;
    }
    if ((fore_item_word & MARIO_STEP_7) > 0)
    {
      bigDispBoard[7][j] = DISP_COLOR_RED;
      bigDispBoard[8][j] = DISP_COLOR_BROWN;
    }
    if ((fore_item_word & MARIO_STEP_8) > 0)
    {
      bigDispBoard[5][j] = DISP_COLOR_RED;
      bigDispBoard[6][j] = DISP_COLOR_BROWN;
    }
    
  }
}







/* Displays mario in current position based on current_mario_row, current_mario_col, current_display_col,
 *                                             hero_face_right, mario_is_big, mario_is_fire, mario_is_luigi, mario_is_walking
 */
void disp_mario(unsigned char mario_color, int current_mario_row, int current_mario_col, int current_display_col)
{
  unsigned char mario_col_now = current_mario_col - current_display_col;
  unsigned char hat_color = MARIO_HAT_COLOR;
  unsigned char pants_color = MARIO_PANTS_COLOR;
  unsigned char hair_color = MARIO_SHOE_COLOR;
  unsigned char shoe_color = MARIO_SHOE_COLOR;
  bool mario_is_jumping = hero_jump_count > 0;
  if (mario_color == PLAY_MARIO_GREEN)
    hat_color = LUIGI_HAT_COLOR;
  else if (mario_color == PLAY_MARIO_PINK)
  {
    hat_color = DISP_COLOR_YELLOW;
    pants_color = DISP_COLOR_PINK;
    shoe_color = DISP_COLOR_LIGHT_PINK;
  }
  else if (mario_color == PLAY_MARIO_ORANGE)
  {
    hat_color = DISP_COLOR_BROWN;
    pants_color = DISP_COLOR_ORANGE;
    shoe_color = DISP_COLOR_YELLOW;
  }
    
  if (mario_is_fire)
  {
    pants_color = DISP_COLOR_RED;
    hat_color = DISP_COLOR_WHITE;
  }
  if (mario_is_star)
  {
    if ((game_count % 6) < 2)
    {
       pants_color = DISP_COLOR_RED;
       hat_color = DISP_COLOR_WHITE;
    }
    else if ((game_count % 6) < 4)
    {
       pants_color = DISP_COLOR_WHITE;
       hat_color = DISP_COLOR_GREEN;
    }
    else 
    {
       pants_color = DISP_COLOR_GREEN;
       hat_color = DISP_COLOR_RED;
    }
  }

  if (current_mario_row > 0)
  {
    if (mario_is_big == false)
    {
      if (hero_face_right == true)
      {
        bigDispBoard[current_mario_row - 1][mario_col_now] = hat_color; // top left pixel
        bigDispBoard[current_mario_row - 1][mario_col_now + 1] = MARIO_FACE_COLOR; // top right pixel
        bigDispBoard[current_mario_row][mario_col_now + 1] = pants_color; // bottom right pixel
        bigDispBoard[current_mario_row][mario_col_now] = pants_color; // bottom left pixel
        /* blink front shoe when walking */
        if ((hero_is_trying == true) && (mario_is_jumping == false) && ((game_count % 8) < 4))
          bigDispBoard[current_mario_row][mario_col_now + 1] = shoe_color; // bottom right pixel
      }
      else
      {
        bigDispBoard[current_mario_row - 1][mario_col_now] = MARIO_FACE_COLOR; // top left pixel
        bigDispBoard[current_mario_row - 1][mario_col_now + 1] = hat_color; // top right pixel
        bigDispBoard[current_mario_row][mario_col_now] = pants_color; // bottom left pixel
        bigDispBoard[current_mario_row][mario_col_now + 1] = pants_color; // bottom right pixel
        /* blink front shoe when walking */
        if ((hero_is_trying == true) && (mario_is_jumping == false) && ((game_count % 8) < 4))
          bigDispBoard[current_mario_row][mario_col_now] = shoe_color; // bottom left pixel
      }
    }
    else /* mario is big */
    {
      if (current_mario_row > 2)
      {
        bigDispBoard[current_mario_row - 3][mario_col_now] = hat_color; // top left pixel
        bigDispBoard[current_mario_row - 3][mario_col_now + 1] = hat_color; // top right pixel
      }
      bigDispBoard[current_mario_row - 1][mario_col_now] = pants_color; // 2/4 left pixel
      bigDispBoard[current_mario_row - 1][mario_col_now + 1] = pants_color; // 2/4 right pixel
      if (hero_face_right == true)
      {
        if (current_mario_row > 1)
        {
          bigDispBoard[current_mario_row - 2][mario_col_now] = hair_color; // 3/4 left pixel
          bigDispBoard[current_mario_row - 2][mario_col_now + 1] = MARIO_FACE_COLOR; // 3/4 right pixel
        }
        bigDispBoard[current_mario_row][mario_col_now] = shoe_color; // bottom left pixel
        /* blink front shoe when walking */
        if ((hero_is_trying == false) || (mario_is_jumping == true) || ((game_count % 8) < 4))
          bigDispBoard[current_mario_row][mario_col_now + 1] = shoe_color; // bottom right pixel
      }
      else
      {
        if (current_mario_row > 1)
        {
          bigDispBoard[current_mario_row - 2][mario_col_now + 1] = hair_color; // 3/4 left pixel
          bigDispBoard[current_mario_row - 2][mario_col_now] = MARIO_FACE_COLOR; // 3/4 right pixel
        }
        bigDispBoard[current_mario_row][mario_col_now + 1] = shoe_color; // bottom left pixel
        /* blink front shoe when walking */
        if ((hero_is_trying == false) || (mario_is_jumping == true) || ((game_count % 8) < 4))
          bigDispBoard[current_mario_row][mario_col_now] = shoe_color; // bottom right pixel
      }
    }
  }
}

/* Applies effects of gravity
 * current_speed is the current speed of the object (-3, 3)
 * initial_count is 
 */
float apply_acceleration(float current_speed, float accel, bool go_to_three)
{
  float max_value = 2.0;
  if (go_to_three)
    max_value = 3.0;
  float result = current_speed + accel;
  if ((current_speed + accel) > max_value)
    result = max_value;
  if ((current_speed + accel) < -max_value)
    result = -max_value;
  return result;
}



/* Determine if (Mario or creature) is on solid ground based on input_row and applicable foreground items (input_col) */
bool on_solid_ground_mariobros(int input_row, int input_col)
{

  bool on_solid_ground = false;
  
  /* Check for bricks */
  if ((input_row == 12) || (input_row == 4))
  {
    unsigned int i;
    for(i = 0; i < NUM_BRICKS; i++)
    {
      if ((input_row == 12) &&  ((locations_low_bricks[i] == input_col) || (locations_low_bricks[i] == input_col + 1) || (locations_low_bricks[i] == input_col - 1)))
        on_solid_ground = true;
      if ((input_row == 4) &&  ((locations_high_bricks[i] == input_col) || (locations_high_bricks[i] == input_col + 1) || (locations_high_bricks[i] == input_col - 1)))
        on_solid_ground = true;
    }

    /* Check for 1UP */
    if ((input_row == 12) && (mario_1up_hit == true) && ((input_col == MARIO_1UP_COL) || (input_col == MARIO_1UP_COL + 1) || (input_col == MARIO_1UP_COL - 1)))
      on_solid_ground = true;
  }
  
  /* Solid ground is non hole ground, on pipe, on blocks, on ?, or on step */
  on_solid_ground = on_solid_ground || (
                          ((input_row == 12) && 
                          (((pgm_read_word_near(&marioDispForeItems[input_col]) & MARIO_LOW_Q) > 0) || 
                          ((pgm_read_word_near(&marioDispForeItems[input_col + 1]) & MARIO_LOW_Q) > 0))) ||

                          ((input_row == 4) && 
                          (((pgm_read_word_near(&marioDispForeItems[input_col]) & MARIO_HIGH_Q) > 0) || 
                          ((pgm_read_word_near(&marioDispForeItems[input_col + 1]) & MARIO_HIGH_Q) > 0))) ||

                          ((input_row == 15) && 
                          (((pgm_read_word_near(&marioDispForeItems[input_col]) & MARIO_LOW_PIPE_TOP) > 0) || 
                          ((pgm_read_word_near(&marioDispForeItems[input_col + 1]) & MARIO_LOW_PIPE_TOP) > 0))) ||

                          ((input_row == 13) && 
                          (((pgm_read_word_near(&marioDispForeItems[input_col]) & MARIO_MED_PIPE_TOP) > 0) || 
                          ((pgm_read_word_near(&marioDispForeItems[input_col + 1]) & MARIO_MED_PIPE_TOP) > 0))) ||

                          ((input_row == 11) && 
                          (((pgm_read_word_near(&marioDispForeItems[input_col]) & MARIO_HIGH_PIPE_TOP) > 0) || 
                          ((pgm_read_word_near(&marioDispForeItems[input_col + 1]) & MARIO_HIGH_PIPE_TOP) > 0))) ||

                          ((input_row == 18) && 
                          (((pgm_read_word_near(&marioDispForeItems[input_col]) & MARIO_STEP_1) > 0) || 
                          ((pgm_read_word_near(&marioDispForeItems[input_col + 1]) & MARIO_STEP_1) > 0))) ||

                          ((input_row == 16) && 
                          (((pgm_read_word_near(&marioDispForeItems[input_col]) & MARIO_STEP_2) > 0) || 
                          ((pgm_read_word_near(&marioDispForeItems[input_col + 1]) & MARIO_STEP_2) > 0))) ||

                          ((input_row == 14) && 
                          (((pgm_read_word_near(&marioDispForeItems[input_col]) & MARIO_STEP_3) > 0) || 
                          ((pgm_read_word_near(&marioDispForeItems[input_col + 1]) & MARIO_STEP_3) > 0))) ||

                          ((input_row == 12) && 
                          (((pgm_read_word_near(&marioDispForeItems[input_col]) & MARIO_STEP_4) > 0) || 
                          ((pgm_read_word_near(&marioDispForeItems[input_col + 1]) & MARIO_STEP_4) > 0))) ||

                          ((input_row == 10) && 
                          (((pgm_read_word_near(&marioDispForeItems[input_col]) & MARIO_STEP_5) > 0) || 
                          ((pgm_read_word_near(&marioDispForeItems[input_col + 1]) & MARIO_STEP_5) > 0))) ||

                          ((input_row == 8) && 
                          (((pgm_read_word_near(&marioDispForeItems[input_col]) & MARIO_STEP_6) > 0) || 
                          ((pgm_read_word_near(&marioDispForeItems[input_col + 1]) & MARIO_STEP_6) > 0))) ||

                          ((input_row == 6) && 
                          (((pgm_read_word_near(&marioDispForeItems[input_col]) & MARIO_STEP_7) > 0) || 
                          ((pgm_read_word_near(&marioDispForeItems[input_col + 1]) & MARIO_STEP_7) > 0))) ||

                          ((input_row == 4) && 
                          (((pgm_read_word_near(&marioDispForeItems[input_col]) & MARIO_STEP_8) > 0) || 
                          ((pgm_read_word_near(&marioDispForeItems[input_col + 1]) & MARIO_STEP_8) > 0))) ||
                          
                          ((input_row == NUM_DISP_ROWS - 2) && 
                          (((pgm_read_word_near(&marioDispForeItems[input_col]) & MARIO_HOLE) == 0) || 
                          ((pgm_read_word_near(&marioDispForeItems[input_col + 1]) & MARIO_HOLE) == 0)))
                          );
                          
   return on_solid_ground;
}

/* Determine if Gimkit on solid ground based on current_hero_row, current_hero_col, applicable foreground items.
 * Applicable items are platforms
 * 
 * Assume platforms are listed from lowest to highest
 * Platforms are identified by row, col of leftmost point => length
 */
bool gimkit_on_solid_ground(int current_hero_row, int current_hero_col)
{
  /* Can't go up when hit platforms*/
  bool on_solid_ground = false;

  /* Check for platforms */
  for(int i = 0; i < NUM_GIMKIT_PLATFORMS; i++)
  {
    if (current_hero_row == pgm_read_byte_near(&gkPlatform_Row[i]) - 2) // gimkit hero two rows less (two above) platform
    {
       if (((current_hero_col + GIMKIT_HERO_WIDTH - 1) >= pgm_read_byte_near(&gkPlatform_Col[i])) &&
           (current_hero_col <= (pgm_read_byte_near(&gkPlatform_Col[i]) + pgm_read_byte_near(&gkPlatform_Length[i]) - 1)))
       {
          on_solid_ground = true;
          gimkit_current_platform = i;
          num_hero_jumps = 0;
       }
          
    }
//    else if (current_hero_row < (pgm_read_byte_near(&gkPlatform_Row[i]) - 2)) // gimkit hero above platform
//      break;
  }

  if (on_solid_ground == true)
    num_hero_jumps = 0;

  return on_solid_ground;
  
}


/* Determine if hero on solid ground based on current_hero_row, current_hero_col, applicable foreground items,
 * Applicable items are bricks and ? blocks
 */
bool on_solid_ground(int current_hero_row, int current_hero_col)
{
  bool return_value = false;
  
  if (game_is_mario == true)
     return_value = on_solid_ground_mariobros(current_hero_row, current_hero_col);
  else
     return_value = gimkit_on_solid_ground(current_hero_row, current_hero_col);

  return return_value;
}


/* Returns bool telling if Mario can go left or right based on input_col and applicable foreground objects (input_row)
 * This includes bricks, ? blocks, pipes, steps
 * Also, mario_is_big and looks at end of level, current_display_col
 * Does not check for collisions with creatures
 */
bool can_go_dir_mariobros(bool is_right, bool is_big, int input_row, int input_col, int current_display_col)
{
  bool can_go_dir = true;
  unsigned char high_row = 13;
  unsigned char low_row = 15;
  if (is_big)
    low_row = 17;

  char adder = -1;
  if (is_right)
    adder = 2;

  unsigned int word1 = pgm_read_word_near(&marioDispForeItems[input_col]);
  unsigned int word2 = pgm_read_word_near(&marioDispForeItems[input_col + adder]);
  unsigned int word3 = pgm_read_word_near(&marioDispForeItems[input_col + 1]);

  /* Check for bricks */
  if ((input_row >= high_row) && (input_row <= low_row))
  {
    unsigned int i;
    for(i = 0; i < NUM_BRICKS; i++)
    {
      if (locations_low_bricks[i] == input_col + adder)
        can_go_dir = false;
    }

    /* Check for 1UP */
    if ((mario_1up_hit == true) && (input_col + adder == MARIO_1UP_COL))
      can_go_dir = false;
  }

  
 
  if (is_right && (input_col >= (NUM_MARIO_COLUMNS - 12))) /* end if level */
    can_go_dir = false;
  else if ((is_right == false) && (input_col == current_display_col)) /* left of screen */
    can_go_dir = false;
  else if (((input_row >= high_row) && (input_row <= low_row)) &&
           ((word2 & (MARIO_LOW_Q | MARIO_HIGH_PIPE_TOP | MARIO_STEP_4)) > 0))
    can_go_dir = false;

  high_row = 5;
  low_row = 7;
  if (is_big)
    low_row = 9;

  /* Check for bricks */
  if ((input_row >= high_row) && (input_row <= low_row))
  {
    unsigned int i;
    for(i = 0; i < NUM_BRICKS; i++)
    {
      if (locations_high_bricks[i] == input_col + adder)
        can_go_dir = false;
    }
  }

  if (((input_row >= high_row) && (input_row <= low_row)) &&
           ((word2 & (MARIO_HIGH_Q | MARIO_STEP_8)) > 0))
    can_go_dir = false;
  else if ((input_row >= 19) &&
           ((word2 & (MARIO_STEP_1)) > 0))
    can_go_dir = false;
  else if ((input_row >= 17) &&
           ((word2 & (MARIO_STEP_2)) > 0))
    can_go_dir = false;
  else if ((input_row >= 15) &&
           ((word2 & (MARIO_STEP_3)) > 0))
    can_go_dir = false;
  else if ((input_row >= 11) &&
           ((word2 & (MARIO_STEP_5)) > 0))
    can_go_dir = false;
  else if ((input_row >= 9) &&
           ((word2 & (MARIO_STEP_6)) > 0))
    can_go_dir = false;
  else if ((input_row >= 7) &&
           ((word2 & (MARIO_STEP_7)) > 0))
    can_go_dir = false;
  
  /* Pipe Base */
  if ((word1 & MARIO_HIGH_PIPE_TOP) > 0)
    high_row = 12;
  if ((word1 & MARIO_MED_PIPE_TOP) > 0)
    high_row = 14;
  if ((word1 & MARIO_LOW_PIPE_TOP) > 0)
    high_row = 16;
  if (((word1 & (MARIO_HIGH_PIPE_TOP | MARIO_MED_PIPE_TOP | MARIO_LOW_PIPE_TOP)) > 0) && 
      ((word2 & (MARIO_HIGH_PIPE_TOP | MARIO_MED_PIPE_TOP | MARIO_LOW_PIPE_TOP)) > 0) &&
      (input_row >= high_row))
    can_go_dir = false;
  else if (is_right)
  {
    /* Pipe Base */
    if ((word2 & MARIO_HIGH_PIPE_TOP) > 0)
      high_row = 12;
    if ((word2 & MARIO_MED_PIPE_TOP) > 0)
      high_row = 14;
    if ((word2 & MARIO_LOW_PIPE_TOP) > 0)
      high_row = 16;
    if (((word3 & (MARIO_HIGH_PIPE_TOP | MARIO_MED_PIPE_TOP | MARIO_LOW_PIPE_TOP)) > 0) && 
      ((word2 & (MARIO_HIGH_PIPE_TOP | MARIO_MED_PIPE_TOP | MARIO_LOW_PIPE_TOP)) > 0) &&
      (input_row >= high_row))
      can_go_dir = false;
  }

  return can_go_dir;
}


/* Determine if Gimkit can go up based on current_hero_row, current_hero_col, direction, applicable foreground items.
 * Applicable items are platforms
 * 
 * Assume platforms are listed from lowest to highest
 * Platforms are identified by row, col of leftmost point => length
 */
bool gimkit_can_go_dir(bool is_right, int current_hero_row, int current_hero_col)
{
  /* can you go? */
  bool can_go_dir = true;

  // NUM_GIMKIT_ROWS

  if ((current_hero_col == 0) && (is_right == false)) // left side of playing area
     can_go_dir = false;
  else if ((current_hero_col == (NUM_GIMKIT_ROWS - GIMKIT_HERO_WIDTH)) && (is_right == true)) // right side of playing area
     can_go_dir = false;
  else
  {
     /* Check for platforms */
     for(int i = 0; i < NUM_GIMKIT_PLATFORMS; i++)
     {
        // if head at or above platform and feet at or below platform (platform is of height 2)
        if (((current_hero_row - GIMKIT_HERO_HEIGHT + 1) <= pgm_read_byte_near(&gkPlatform_Row[i])) &&
            (current_hero_row >= (pgm_read_byte_near(&gkPlatform_Row[i]) - 1)))
        {
           if (((current_hero_col + GIMKIT_HERO_WIDTH) == pgm_read_byte_near(&gkPlatform_Col[i])) && (is_right == true)) // one to the left and facing right
              can_go_dir = false;
           else if ((current_hero_col == (pgm_read_byte_near(&gkPlatform_Col[i]) + pgm_read_byte_near(&gkPlatform_Length[i]))) && (is_right == true)) // one to the right and facing left
              can_go_dir = false;
        }
//        else if (current_hero_row < pgm_read_byte_near(&gkPlatform_Row[i])) // gimkit hero above platform
//           break;
      }
  }

  return can_go_dir;
  
}


bool hero_can_go_dir(bool is_right, bool is_big, int input_row, int input_col, int current_display_col)
{
  bool return_value = false;
  
  if (game_is_mario == true)
     return_value = can_go_dir_mariobros(is_right, is_big, input_row, input_col, current_display_col);
  else
     return_value = gimkit_can_go_dir(is_right, input_row, input_col);

  return return_value;
}



#define MARIO_GOODBYE_GOOMBA 1
#define MARIO_GOODBYE_KOOPA 2
#define MARIO_GOODBYE_MARIO 3

float goodbye_row = 0.0;
float goodbye_col = 0.0;
float goodbye_vert_speed = 0.0;
char goodbye_character = 0;
char goodbye_char_dir = MOVE_NONE;

void set_goodbye_char(int input_row, int input_col, char character, char char_direction)
{
  goodbye_row = (float)input_row;
  goodbye_col = (float)input_col;
  goodbye_character = character;
  goodbye_char_dir = char_direction;
  goodbye_vert_speed = 2.0;
  total_score++;
}

void display_goodbye_char(unsigned char mario_color, int current_display_col)
{
  if (goodbye_col > 0.5)
  {
    int char_col_now = (int)goodbye_col - current_display_col;
    int char_row_now = (int)goodbye_row;
    if (goodbye_character == MARIO_GOODBYE_MARIO)
    {
      bigDispBoard[char_row_now][char_col_now] = DISP_COLOR_BLACK;
      bigDispBoard[char_row_now][char_col_now + 1] = DISP_COLOR_BLACK;
      bigDispBoard[char_row_now - 1][char_col_now] = DISP_COLOR_BLACK;
      bigDispBoard[char_row_now - 1][char_col_now + 1] = DISP_COLOR_BLACK;
    }
    
    /* update position */
    goodbye_vert_speed = goodbye_vert_speed - HERO_ACCELERATION;
    if (goodbye_vert_speed < -3.0)
      goodbye_vert_speed = -3.0;

    /* Update char row position */
    goodbye_row = goodbye_row - goodbye_vert_speed / 2;
    
    /* Update char col position */
    if (goodbye_char_dir == MOVE_RIGHT)
      goodbye_col = goodbye_col + 1.0 / 3;
    else if (goodbye_char_dir == MOVE_LEFT)
      goodbye_col = goodbye_col - 1.0 / 3;

    /* Display character */
    char_col_now = (int)goodbye_col - current_display_col;
    char_row_now = (int)goodbye_row;
    if ((char_col_now >= 0) && (char_col_now < 22) && (char_row_now >= 0) && (char_row_now < 22) )
    {
      if (goodbye_character == MARIO_GOODBYE_GOOMBA)
      {
        bigDispBoard[char_row_now][char_col_now] = DISP_COLOR_RED;
        bigDispBoard[char_row_now][char_col_now + 1] = DISP_COLOR_RED;
        bigDispBoard[char_row_now - 1][char_col_now] = DISP_COLOR_WHITE;
        bigDispBoard[char_row_now - 1][char_col_now + 1] = DISP_COLOR_WHITE;
      }
      else if (goodbye_character == MARIO_GOODBYE_KOOPA)
      {
        bigDispBoard[char_row_now][char_col_now] = DISP_COLOR_GREEN;
        bigDispBoard[char_row_now][char_col_now + 1] = DISP_COLOR_GREEN;
        bigDispBoard[char_row_now - 1][char_col_now] = DISP_COLOR_WHITE;
        bigDispBoard[char_row_now - 1][char_col_now + 1] = DISP_COLOR_WHITE;
      }
      else if (goodbye_character == MARIO_GOODBYE_MARIO)
      {
        unsigned char hat_color = MARIO_HAT_COLOR;
        unsigned char pants_color = MARIO_PANTS_COLOR;
        unsigned char hair_color = MARIO_SHOE_COLOR;
 
        if (mario_color == PLAY_MARIO_GREEN)
          hat_color = LUIGI_HAT_COLOR;
        else if (mario_color == PLAY_MARIO_PINK)
        {
          hat_color = DISP_COLOR_YELLOW;
          pants_color = DISP_COLOR_PINK;
        }
        else if (mario_color == PLAY_MARIO_ORANGE)
        {
          hat_color = DISP_COLOR_BROWN;
          pants_color = DISP_COLOR_ORANGE;
        }
        
        bigDispBoard[char_row_now][char_col_now] = pants_color;
        bigDispBoard[char_row_now][char_col_now + 1] = pants_color;
        bigDispBoard[char_row_now - 1][char_col_now] = hat_color;
        bigDispBoard[char_row_now - 1][char_col_now + 1] = MARIO_FACE_COLOR;
      }
    }
    else
      goodbye_col = 0.0;
      
  } /* animation active */
}



unsigned int fireball_col[2] = {0,0};
unsigned char fireball_row[2] = {0,0};
int fireball_dir[2] = {1,1};
unsigned int fireball_count = 0;

unsigned int brick_bump_break_col = 0;
unsigned int brick_bump_break_count = 0;


#define GOOMBA_COL_1  50
#define GOOMBA_COL_2  88
#define GOOMBA_COL_3  104
#define GOOMBA_COL_4  108
#define GOOMBA_COL_5  206
#define GOOMBA_COL_6  210
#define GOOMBA_COL_7  242
#define GOOMBA_COL_8  246
#define GOOMBA_COL_9  262
#define GOOMBA_COL_10 266
#define GOOMBA_COL_11 272 
#define GOOMBA_COL_12 276
#define GOOMBA_COL_13 364
#define GOOMBA_COL_14 368

#define HIGH_GOOMBA_COL_1 172
#define HIGH_GOOMBA_COL_2 176

#define KOOPA_COL 254

/* up to 4 active goombas */
float goomba_row[4] = {0.0,0.0,0.0,0.0};
float goomba_col[4] = {0.0,0.0,0.0,0.0};
bool goomba_face_right[4] = {false, false, false, false};
unsigned int goomba_col_done = 0;

/* returns True if Goomba got Mario */
bool display_goombas(int current_mario_row, int current_mario_col, int current_display_col, int current_mario_jump_speed) 
{
  bool return_value = false;
  float new_goomba_col = 0.0;
  float new_goomba_row = 0.0;
  if (current_display_col == GOOMBA_COL_1 - 20)
    new_goomba_col = GOOMBA_COL_1;
  else if (current_display_col == GOOMBA_COL_2 - 20)
    new_goomba_col = GOOMBA_COL_2;
  else if (current_display_col == GOOMBA_COL_3 - 20)
    new_goomba_col = GOOMBA_COL_3;
  else if (current_display_col == GOOMBA_COL_4 - 20)
    new_goomba_col = GOOMBA_COL_4;
  else if (current_display_col == GOOMBA_COL_5 - 20)
    new_goomba_col = GOOMBA_COL_5;
  else if (current_display_col == GOOMBA_COL_6 - 20)
    new_goomba_col = GOOMBA_COL_6;
  else if (current_display_col == GOOMBA_COL_7 - 20)
    new_goomba_col = GOOMBA_COL_7;
  else if (current_display_col == GOOMBA_COL_8 - 20)
    new_goomba_col = GOOMBA_COL_8;
  else if (current_display_col == GOOMBA_COL_9 - 20)
    new_goomba_col = GOOMBA_COL_9;
  else if (current_display_col == GOOMBA_COL_10 - 20)
    new_goomba_col = GOOMBA_COL_10;
  else if (current_display_col == GOOMBA_COL_11 - 20)
    new_goomba_col = GOOMBA_COL_11;
  else if (current_display_col == GOOMBA_COL_12 - 20)
    new_goomba_col = GOOMBA_COL_12;
  else if (current_display_col == GOOMBA_COL_13 - 20)
    new_goomba_col = GOOMBA_COL_13;
  else if (current_display_col == GOOMBA_COL_14 - 20)
    new_goomba_col = GOOMBA_COL_14;

  if (new_goomba_col > 0.0)
    new_goomba_row = 20.0;

  if (current_display_col == HIGH_GOOMBA_COL_1 - 20)
  {
    new_goomba_col = HIGH_GOOMBA_COL_1;
    new_goomba_row = 4.0;
  }
  else if (current_display_col == HIGH_GOOMBA_COL_2 - 20)
  {
    new_goomba_col = HIGH_GOOMBA_COL_2;
    new_goomba_row = 4.0;
  }

  if ((unsigned int)new_goomba_col == goomba_col_done)
  {
    new_goomba_col = 0.0;
    new_goomba_row = 0.0;
  }

  /* new goomba */
  if (new_goomba_col > 0.0)
  {
    goomba_col_done = (unsigned int)new_goomba_col;
    if (goomba_col[0] < 1.0)
    {
      goomba_col[0] = new_goomba_col;
      goomba_row[0] = new_goomba_row;
      goomba_face_right[0] = false;
    }
    else if (goomba_col[1] < 1.0)
    {
      goomba_col[1] = new_goomba_col;
      goomba_row[1] = new_goomba_row;
      goomba_face_right[1] = false;
    }
    else if (goomba_col[2] < 1.0)
    {
      goomba_col[2] = new_goomba_col;
      goomba_row[2] = new_goomba_row;
      goomba_face_right[2] = false;
    }
    else if (goomba_col[3] < 1.0)
    {
      goomba_col[3] = new_goomba_col;
      goomba_row[3] = new_goomba_row;
      goomba_face_right[3] = false;
    }
  }

  for(unsigned int i = 0; i < 4; i++)
  {
    /* Update goomba positions */
    if (goomba_col[i] > 0.5) /* active goomba */
    {
      int goomba_col_now = (int)goomba_col[i] - current_display_col;
      /* Horizontal */
      if ((can_go_dir_mariobros(goomba_face_right[i], false, (int)goomba_row[i], (int)goomba_col[i], current_display_col) == false) && (goomba_col_now != 0))
        goomba_face_right[i] = !goomba_face_right[i];
      if (goomba_face_right[i] == true)
        goomba_col[i] = goomba_col[i] + 1.0 / 5; /* goomba col */
      else
        goomba_col[i] = goomba_col[i] - 1.0 / 5; /* goomba col */

      /* Vertical */
      if (on_solid_ground((int)goomba_row[i], (int)goomba_col[i]) == false)
        goomba_row[i] = goomba_row[i] + 3.0 / 5; /* goomba row */
      
   

      /* Display goombas */
      goomba_col_now = (int)goomba_col[i] - current_display_col;
      int goomba_row_now = (int)goomba_row[i];
      if ((goomba_col_now >= 0) && (goomba_col_now < 21) && (goomba_row_now < 22))
      {
        bigDispBoard[goomba_row_now - 1][goomba_col_now] = DISP_COLOR_RED;
        bigDispBoard[goomba_row_now - 1][goomba_col_now + 1] = DISP_COLOR_RED;
        if ((game_count % 20) < 10)
          bigDispBoard[goomba_row_now][goomba_col_now] = DISP_COLOR_WHITE;
        else
          bigDispBoard[goomba_row_now][goomba_col_now + 1] = DISP_COLOR_WHITE;
      }

      /* Stomp, fireball, star, bump/smash, koopa kick */
      /* Check for goomba gets mario */
      /* Check for mario gets goomba, star, stomp, fireball */
      int mario_col_now = current_mario_col - current_display_col;
      if (((current_mario_row == goomba_row_now) || ((current_mario_row - 1) == goomba_row_now) || ((current_mario_row + 1) == goomba_row_now)) && 
        ((mario_col_now == goomba_col_now) || ((mario_col_now + 1) == goomba_col_now) || ((mario_col_now - 1) == goomba_col_now)))
      {
        if (mario_is_star)
        {
          char char_direction = MOVE_RIGHT;
          if (goomba_face_right[i] == false)
            char_direction = MOVE_LEFT;
          set_goodbye_char((int)goomba_row[i], (int)goomba_col[i], MARIO_GOODBYE_GOOMBA, char_direction);
          goomba_col[i] = 0.0; /* remove the goomba */
          delay(100);
        }
        else if ((game_count - mario_invulnerable_count) > MARIO_INVUL_COUNT)
        {
          if (mario_is_fire)
            mario_is_fire = false;
          else if (mario_is_big)
            mario_is_big = false;
          else
            return_value = true;

          if (return_value == false)
          {
            mario_invulnerable_count = game_count;
            delay(100);
          }
        }
      }
      else if ((current_mario_row == goomba_row_now - 2) && (current_mario_jump_speed < 0.0) &&
               ((mario_col_now == goomba_col_now) || ((mario_col_now + 1) == goomba_col_now) || ((mario_col_now - 1) == goomba_col_now)))
      {
        /* stomp */
        goomba_col[i] = 0.0; /* remove the goomba */
        total_score++;
        delay(100);
      }
      else if (((fireball_row[0] == goomba_row_now) || (fireball_row[0] == (goomba_row_now - 1))) && 
               ((fireball_col[0] - current_display_col) == goomba_col_now))
      {
        char char_direction = MOVE_RIGHT;
        if (goomba_face_right[i] == false)
          char_direction = MOVE_LEFT;
        set_goodbye_char((int)goomba_row[i], (int)goomba_col[i], MARIO_GOODBYE_GOOMBA, char_direction);
        fireball_col[0] = 0;
        goomba_col[i] = 0.0; /* remove the goomba */
      }
      else if (((fireball_row[1] == goomba_row_now) || (fireball_row[1] == (goomba_row_now - 1))) && 
               ((fireball_col[1] - current_display_col) == goomba_col_now))
      {
        char char_direction = MOVE_RIGHT;
        if (goomba_face_right[i] == false)
          char_direction = MOVE_LEFT;
        set_goodbye_char((int)goomba_row[i], (int)goomba_col[i], MARIO_GOODBYE_GOOMBA, char_direction);
        fireball_col[1] = 0;
        goomba_col[i] = 0.0; /* remove the goomba */
      }
      else if (((goomba_row_now == 12) || (goomba_row_now == 11)) && ((game_count - brick_bump_break_count) < 10) &&
               (((brick_bump_break_col - current_display_col) == goomba_col_now) || ((brick_bump_break_col - current_display_col - 1) == goomba_col_now) || ((brick_bump_break_col - current_display_col + 1) == goomba_col_now)))
      {
        char char_direction = MOVE_RIGHT;
        if (goomba_face_right[i] == false)
          char_direction = MOVE_LEFT;
        set_goodbye_char((int)goomba_row[i], (int)goomba_col[i], MARIO_GOODBYE_GOOMBA, char_direction);
        goomba_col[i] = 0.0; /* remove the goomba */
      }

      if (goomba_col_now < -20)
        goomba_col[i] = 0.0; /* remove the goomba */
    

    }

  }
  

  return return_value;

}













/* koopa troopa */
float koopa_col = 0.0;
bool koopa_moving = true; /* true if koopa walking, not stomped */
bool koopa_kicked_right = false;
bool koopa_kicked_left = false;
unsigned int koopa_count = 0;

/* returns True if Koopa got Mario */
bool display_koopa(int current_mario_row, int current_mario_col, int current_display_col, float *current_mario_jump_speed) 
{
  bool return_value = false;
  int mario_col_now = current_mario_col - current_display_col;

  /* check if koopa active */
  if (current_display_col == KOOPA_COL - 20)
    koopa_col = KOOPA_COL;

  /* Update koopa position */
  if (koopa_col > 0.5) /* active koopa */
  {
    int koopa_col_now = (int)koopa_col - current_display_col;
    /* Horizontal */
    if (koopa_moving) /* always walking left, don't need to check for obstacles while walking */
    {
      /* update position */
      koopa_col = koopa_col - 1.0 / 4; /* goomba col */  
      koopa_col_now = (int)koopa_col - current_display_col;

      /* display koopa head */
      bigDispBoard[18][koopa_col_now] = DISP_COLOR_Q_YELLOW;

      /* check for koopa get mario, star, stomp */
      int mario_col_now = current_mario_col - current_display_col;
      if (((current_mario_row == 20) || (current_mario_row == 19)) && 
        ((mario_col_now == koopa_col_now) || ((mario_col_now + 1) == koopa_col_now) || ((mario_col_now - 1) == koopa_col_now)))
      {
        if (mario_is_star)
        {
          set_goodbye_char(20, (int)koopa_col, MARIO_GOODBYE_KOOPA, MOVE_LEFT);
          koopa_col = 0.0; /* remove koopa */
          delay(100);
        }
        else if ((game_count - mario_invulnerable_count) > MARIO_INVUL_COUNT)
        {
          if (mario_is_fire)
            mario_is_fire = false;
          else if (mario_is_big)
            mario_is_big = false;
          else
            return_value = true;

          if (return_value == false)
          {
            mario_invulnerable_count = game_count;
            delay(100);
          }
        }
      }
      else if ((current_mario_row == 18) && ((*current_mario_jump_speed) < 0.0) &&
                ((mario_col_now == koopa_col_now) || ((mario_col_now + 1) == koopa_col_now) || ((mario_col_now - 1) == koopa_col_now)))
      {
        koopa_moving = false; /* koopa stops moving */
        koopa_kicked_right = false;
        koopa_kicked_left = false;
        koopa_count = game_count;
        (*current_mario_jump_speed) = 2.0;
      }
    } /* koopa moving */
    else if ((koopa_kicked_right == false) && (koopa_kicked_left == false))
    { /* koopa shell */
      if (((current_mario_row == 18) || (current_mario_row == 19) || (current_mario_row == 20)) && ((mario_col_now == koopa_col_now) || (mario_col_now == (koopa_col_now - 1)) || (mario_col_now == (koopa_col_now - 2))))
      {
        koopa_kicked_right = true;
        mario_invulnerable_count = game_count;
      }
      else if (((current_mario_row == 18) || (current_mario_row == 19) || (current_mario_row == 20)) && (mario_col_now == (koopa_col_now + 1) || ((mario_col_now + 1) == (koopa_col_now + 2))))
      {
        koopa_kicked_left = true;
        mario_invulnerable_count = game_count;
      }
      else if ((koopa_count > 0) && ((game_count - koopa_count) > 100))
      {
        koopa_moving = true;
        koopa_count = 0;
      }
    }
    else
    { /* koopa has been kicked */
      if ((koopa_kicked_right) && (can_go_dir_mariobros(true, false, 20, (int)koopa_col, current_display_col) == false))
      {
        koopa_kicked_right = false;
        koopa_kicked_left = true;
      }

      /* Update position */
      if (koopa_kicked_right)
        koopa_col = koopa_col + 3.0 / 3; /* koopa col */
      else if (koopa_kicked_left)
        koopa_col = koopa_col - 3.0 / 3; /* koopa col */
      koopa_col_now = (int)koopa_col - current_display_col;

      /* Check for hit goomba, mario, or re-stomp */
      if ((current_mario_row == 18) && 
                ((mario_col_now == koopa_col_now) || ((mario_col_now + 1) == koopa_col_now) || ((mario_col_now - 1) == koopa_col_now)))
      {
        koopa_moving = false; /* koopa stops moving */
        koopa_kicked_right = false;
        koopa_kicked_left = false;
        koopa_count = game_count;
        (*current_mario_jump_speed) = 2.0;
      }
      else if (((current_mario_row == 20) || (current_mario_row == 19)) && 
        ((mario_col_now == koopa_col_now) || ((mario_col_now + 1) == koopa_col_now) || ((mario_col_now - 1) == koopa_col_now)))
      {
        if (mario_is_star)
        {
          set_goodbye_char(20, (int)koopa_col, MARIO_GOODBYE_KOOPA, MOVE_LEFT);
          koopa_col = 0.0; /* remove koopa */
          delay(100);
        }
        else if ((game_count - mario_invulnerable_count) > MARIO_INVUL_COUNT)
        {
          if (mario_is_fire)
            mario_is_fire = false;
          else if (mario_is_big)
            mario_is_big = false;
          else
            return_value = true;

          if (return_value == false)
          {
            mario_invulnerable_count = game_count;
            delay(100);
          }
        }
      }

      /* Check for goombas */
      for(unsigned int i = 0; i < 4; i++)
      {
        int goomba_col_now = goomba_col[i] - current_display_col;
        if ((koopa_col_now == goomba_col_now) || ((koopa_col_now + 1) == goomba_col_now) || ((koopa_col_now - 1) == goomba_col_now))
        {
          set_goodbye_char(20, (int)koopa_col, MARIO_GOODBYE_GOOMBA, MOVE_LEFT);
          goomba_col[i] = 0.0; /* remove the goomba */
        }
      }
    } /* kicked koopa */

    if ((koopa_col_now == 0) || (koopa_col_now == 21))
      koopa_col = 0.0; /* remove koopa */
    else if (((fireball_row[0] == 20) || (fireball_row[0] == 19)) && 
               ((fireball_col[0] - current_display_col) == koopa_col_now))
    {
      set_goodbye_char(20, (int)koopa_col, MARIO_GOODBYE_KOOPA, MOVE_LEFT);
      koopa_col = 0.0; /* remove koopa */
      fireball_col[0] = 0;
    }
    else if (((fireball_row[1] == 20) || (fireball_row[1] == 19)) && 
             ((fireball_col[1] - current_display_col) == koopa_col_now))
    {
      set_goodbye_char(20, (int)koopa_col, MARIO_GOODBYE_KOOPA, MOVE_LEFT);
      koopa_col = 0.0; /* remove koopa */
      fireball_col[1] = 0;
    }

    /* Display koopa body */
    bigDispBoard[20][koopa_col_now] = DISP_COLOR_WHITE;
    bigDispBoard[20][koopa_col_now + 1] = DISP_COLOR_WHITE;
    bigDispBoard[19][koopa_col_now] = DISP_COLOR_GREEN;
    bigDispBoard[19][koopa_col_now + 1] = DISP_COLOR_GREEN;   

  } /* active koopa */

  return return_value;

}






unsigned int coin_row = 0;
unsigned int coin_col = 0;
unsigned int coin_count = 0;
void set_coin_animation(unsigned char input_row, unsigned int input_col)
{
  coin_row = input_row;
  coin_col = input_col;
  coin_count = game_count;
  total_score++;
}

void display_coin_animation(int current_display_col)
{
  unsigned int coin_disp_col = coin_col - current_display_col;
  
  /* Update coin position */
  if ((coin_row > 0) && ((game_count - coin_count) < 12))
  {
    coin_row--;
  }
  else
  {
    coin_row = 0;
    coin_count = 0;
  }
  
  /* Display coin */
  
  if ((coin_disp_col >= 0) && (coin_disp_col < 21) && (coin_row > 0))
  {
    if (((game_count - coin_count) % 8) < 4)
    {
      bigDispBoard[coin_row][coin_disp_col] = DISP_COLOR_WHITE;
      bigDispBoard[coin_row - 1][coin_disp_col] = DISP_COLOR_WHITE;
      bigDispBoard[coin_row][coin_disp_col + 1] = DISP_COLOR_YELLOW;
      bigDispBoard[coin_row - 1][coin_disp_col + 1] = DISP_COLOR_YELLOW;
    }
    else
    {
      bigDispBoard[coin_row][coin_disp_col] = DISP_COLOR_YELLOW;
      bigDispBoard[coin_row - 1][coin_disp_col] = DISP_COLOR_YELLOW;
      bigDispBoard[coin_row][coin_disp_col + 1] = DISP_COLOR_WHITE;
      bigDispBoard[coin_row - 1][coin_disp_col + 1] = DISP_COLOR_WHITE;
    }
  }
}


void set_mush_fire(unsigned char input_row, unsigned int input_col, bool mush_is_green)
{
  mush_row = input_row;
  mush_col = input_col;
  mush_count = game_count;
  mush_is_red = !mush_is_green;
  mush_go_right = true;
  if ((mario_is_big) && (mush_is_green == false))
    mush_is_flower = true;
  else
    mush_is_flower = false;
  brick_bump_break_col = input_col;
  brick_bump_break_count = game_count;
}

void display_mush(int current_display_col, int current_mario_row, int current_mario_col)
{
  if (mush_is_flower == false)
  {
    /* Update mush position */
    if (((((mush_row == 14) || (mush_row == 13)) && ((mush_col == low_mush_fire[0]) || (mush_col == low_mush_fire[1]) || (mush_col == MARIO_1UP_COL)))
              || (((mush_row == 5) || (mush_row == 6)) && (mush_col == high_mush_fire))) && ((game_count - mush_count) < 34)) /* going up */
    {
      brick_bump_break_col = 0;
      if (((game_count - mush_count) % 3) == 0)
        mush_row--;
    }
    else 
    {
      if (((game_count - mush_count) % 3) == 0) /* time to move */
      {
        if (can_go_dir_mariobros(mush_go_right, false, mush_row, mush_col, current_display_col) == false) /* can't go this way, go other way */
          mush_go_right = !mush_go_right;

        /* Check for bumps */
        if ((mush_row == 12) && ((game_count - brick_bump_break_count) < 10) && (mush_go_right == true) &&
               ((brick_bump_break_col == mush_col) || (brick_bump_break_col == (mush_col + 1) || (brick_bump_break_col == (mush_col + 2)))))
          mush_go_right = false;
        else if ((mush_row == 12) && ((game_count - brick_bump_break_count) < 10) && (mush_go_right == false) &&
               ((brick_bump_break_col == (mush_col - 1)) || (brick_bump_break_col == (mush_col - 2))))
          mush_go_right = true;
      
        if (mush_go_right) /* move horizontally */
          mush_col++;
        else
          mush_col--;
      }

      if ((((game_count - mush_count) % 2) == 0) && (on_solid_ground(mush_row, mush_col) == false)) /* dropping */ 
        mush_row++;
    }

    unsigned int mush_disp_col = mush_col - current_display_col;  

  
    /* Check for Mario eating mush */
    if (((current_mario_row == mush_row) || ((current_mario_row - 1) == mush_row) || ((current_mario_row + 1) == mush_row)) && 
        ((current_mario_col == mush_col) || ((current_mario_col + 1) == mush_col) || ((current_mario_col - 1) == mush_col)) && (mush_count != 0))
    {
      if (mush_is_red)
        mario_is_big = true;
      else 
        mario_lives++;
      mush_count = 0; /* eaten */
      mush_go_right = true; /* for next mush */
      mush_row = 0;
      mush_col = 0;
      mush_is_red = true; /* assume next is red */
      total_score++;
      delay(600);
    }
    else if ((mush_count > 0) && (mush_disp_col > 0) && (mush_disp_col < 21) && (mush_row > 0) && (mush_row < 21)) /* Display mush */
    {
      bigDispBoard[mush_row][mush_disp_col] = DISP_COLOR_WHITE;
      bigDispBoard[mush_row - 1][mush_disp_col] = DISP_COLOR_Q_YELLOW;
      bigDispBoard[mush_row][mush_disp_col + 1] = DISP_COLOR_WHITE;
      if (mush_is_red)
        bigDispBoard[mush_row - 1][mush_disp_col + 1] = DISP_COLOR_RED;
      else
        bigDispBoard[mush_row - 1][mush_disp_col + 1] = DISP_COLOR_GREEN;
    }
    else if ((mush_disp_col == 0) || (mush_row == 21))
    {
      mush_count = 0; /* out of bounds */
      mush_go_right = true; /* for next mush */
    }
  } /* mush is mush */
  else
  { /* mush is flower */
    /* Update flower position */
    if (((((mush_row == 14) || (mush_row == 13)) && ((mush_col == low_mush_fire[0]) || (mush_col == low_mush_fire[1])))
              || (((mush_row == 5) || (mush_row == 6)) && (mush_col == high_mush_fire))) && ((game_count - mush_count) < 34)) /* going up */
      if (((game_count - mush_count) % 16) == 0)
        mush_row--;

    /* For Display flower */
    unsigned int mush_disp_col = mush_col - current_display_col;  
  
    /* Check for Mario eating flower */
    if (((current_mario_row == mush_row) || ((current_mario_row - 1) == mush_row) || ((current_mario_row + 1) == mush_row)) && 
        ((current_mario_col == mush_col) || ((current_mario_col + 1) == mush_col) || ((current_mario_col - 1) == mush_col)) && (mush_count != 0))
    {
      if (mario_is_big == false)
        mario_is_big = true;
      else 
        mario_is_fire = true;
      mush_count = 0; /* eaten */
      mush_row = 0;
      mush_col = 0;
      total_score++;
      delay(600);
    }
    else if ((mush_count > 0) && (mush_disp_col > 0) && (mush_disp_col < 21)) /* Display flower */
    {
      unsigned char right_flower_color = DISP_COLOR_WHITE;
      unsigned char left_flower_color = DISP_COLOR_YELLOW;
      if (((game_count - mush_count) % 6) < 2)
      {
        right_flower_color = DISP_COLOR_YELLOW;
        left_flower_color = DISP_COLOR_RED;
      }
      else if (((game_count - mush_count) % 6) < 4)
      {
        right_flower_color = DISP_COLOR_RED;
        left_flower_color = DISP_COLOR_WHITE;
      }
      bigDispBoard[mush_row - 1][mush_disp_col] = left_flower_color;
      bigDispBoard[mush_row - 1][mush_disp_col + 1] = right_flower_color;
      bigDispBoard[mush_row][mush_disp_col + 1] = DISP_COLOR_GREEN;
    }
  }

}


/* Store breaking bricks for animation */
/* Stores initial row, col.  Time updates and determines current display and whether active */
float breaking_brick_row[2][4] = {{22.0,22.0,22.0,22.0},{22.0,22.0,22.0,22.0}}; /* brick, (0-top left brick col,1-bottom left brick row, 2-top right brick col,1-bottom right brick row) */
float breaking_brick_col[2][4] = {{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0}}; /* brick, (0-top left brick col,1-bottom left brick row, 2-top right brick col,1-bottom right brick row) */
float breaking_brick_vert_speed[2][4] = {{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0}}; /* brick, (0-top left brick col,1-bottom left brick row, 2-top right brick col,1-bottom right brick row) */
bool bumping_brick = false; /* true if breaking, false if bumping */
unsigned char bump_row = 0;
unsigned int bump_col = 0;

void set_breaking_brick(unsigned char input_row, unsigned int input_col, bool bump_brick)
{
  unsigned char location = 0;
  if (breaking_brick_row[1][0] < 22.0)
    location = 0;
  else if (breaking_brick_row[0][0] < 22.0)
    location = 1;

  brick_bump_break_col = input_col;
  brick_bump_break_count = game_count;
  Serial.println(brick_bump_break_count);

  if (bump_brick == false)
  {
    breaking_brick_row[location][0] = input_row - 1;
    breaking_brick_row[location][1] = input_row;
    breaking_brick_row[location][2] = input_row - 1;
    breaking_brick_row[location][3] = input_row;
    breaking_brick_col[location][0] = input_col;
    breaking_brick_col[location][1] = input_col;
    breaking_brick_col[location][2] = input_col + 1;
    breaking_brick_col[location][3] = input_col + 1;
    /* Set initial vertical speeds */
    breaking_brick_vert_speed[location][0] = 2.0;
    breaking_brick_vert_speed[location][1] = 1.0;
    breaking_brick_vert_speed[location][2] = 2.0;
    breaking_brick_vert_speed[location][3] = 1.0;
    bumping_brick = false;
    mario_bump_brick_count = 0;
  }
  else
  {
    bumping_brick = true;
    bump_row = input_row;
    bump_col = input_col;
    mario_bump_brick_count = game_count;
  }
}

void disp_breaking_brick(int current_display_col)
{
  /* Display the 4 brick fragments */
  /* bottom bricks have upward, outward speeds of 1
   * top bricks have upward, outward speeds of 2
   */

  /* update rate based on current brick speed */
  unsigned char i, j, k;

  /* Update brick vertical speeds */
  for(i = 0; i < 2; i++)
  {
    if (bumping_brick == false) /* not a bump */
    {
      for(j = 0; j < 4; j++) 
      {
        if (breaking_brick_row[i][j] < 22.0) /* active bricks in top row */
        {
          breaking_brick_vert_speed[i][j] = breaking_brick_vert_speed[i][j] - HERO_ACCELERATION;
          if (breaking_brick_vert_speed[i][j] < -3.0)
            breaking_brick_vert_speed[i][j] = -3.0;

          /* Update brick row position */
          breaking_brick_row[i][j] = breaking_brick_row[i][j] - breaking_brick_vert_speed[i][j] / 4;
        }
      }
      /* Update brick col position */
      if (breaking_brick_row[i][0] < 22.0) /* active bricks in top row */
      {
        breaking_brick_col[i][0] = breaking_brick_col[i][0] - 2.0 / 4; /* top left brick col */
        breaking_brick_col[i][1] = breaking_brick_col[i][1] - 1.0 / 4; /* bottom left brick col */
        breaking_brick_col[i][2] = breaking_brick_col[i][2] + 2.0 / 4; /* top right brick col */
        breaking_brick_col[i][3] = breaking_brick_col[i][3] + 1.0 / 4; /* bottom right brick col */
      }

      /* Display breaking bricks */
      int brick_col_now = 0;
      int brick_row_now = 0;
      for(j = 0; j < 4; j++)
      {
        brick_row_now = breaking_brick_row[i][j];
        brick_col_now = breaking_brick_col[i][j] - current_display_col;
        if ((brick_col_now >= 0) && (brick_col_now < 22) && (brick_row_now >= 0) && (brick_row_now < 22) )
          bigDispBoard[brick_row_now][brick_col_now] = DISP_COLOR_HALF_RED;
      }
      
    } 
  }

  /* not a bump */
  if ((game_count - mario_bump_brick_count) < 8)
  { /* just a bump */
    unsigned int bump_disp_col = bump_col - current_display_col;
    if (bump_disp_col < 21)
    {
      bigDispBoard[bump_row][bump_disp_col] = DISP_COLOR_BLACK;
      bigDispBoard[bump_row - 2][bump_disp_col] = DISP_COLOR_HALF_RED;
    }
    if ((bump_disp_col + 1) < 21)
    {
      bigDispBoard[bump_row][bump_disp_col + 1] = DISP_COLOR_BLACK;
      bigDispBoard[bump_row - 2][bump_disp_col + 1] = DISP_COLOR_HALF_RED;
    }
  }
  else if (mario_bump_brick_count > 0)
  {
    bumping_brick = false;
    mario_bump_brick_count = 0;
  }

}

bool fireball_been_pressed = false;

/* Manages creation, propagation, and display of fireballs */
void display_mario_fireballs(int current_display_col, int current_mario_row, int current_mario_col, unsigned char button_press)
{
  
  bool fireball_pressed = false;
  if (((button_press & MOVE_ROTATE_LEFT) > 0) && (fireball_been_pressed == false)) /* B Button pressed */
  {
    fireball_pressed = true;
    fireball_been_pressed = true;
  }
  else if ((button_press & MOVE_ROTATE_LEFT) == 0)
    fireball_been_pressed = false;

  unsigned char location = 0;
  if (fireball_col[0] > 0)
    location = 1;
  if (fireball_col[1] > 0)
    location = 2;

  if ((mario_is_fire) && (fireball_pressed) && (location < 2) && ((game_count - fireball_count) > 8)) /* open slot for fireball */
  {
    fireball_row[location] = current_mario_row;
    fireball_col[location] = current_mario_col;
    fireball_count = game_count;
    if (hero_face_right == true)
      fireball_dir[location] = 1;
    else
      fireball_dir[location] = -1;
  }

  /* update fireball locations */
  //if ((game_count % 2) == 0)
    for (unsigned int i = 0; i < 2; i++)
    {
      if (fireball_col[i] > 0) /* fireball is active */
      {
        bool fire_is_right = (fireball_dir[location] > 0);
        if (can_go_dir_mariobros(fire_is_right, false, fireball_row[i], fireball_col[i], current_display_col))
          fireball_col[i] = fireball_col[i] + fireball_dir[i];
        else
          fireball_col[i] = 0;
        
        if (on_solid_ground(fireball_row[i], fireball_col[i]))
          fireball_row[i] = fireball_row[i] - 1;
        else if (fireball_row[i] < 21)
          fireball_row[i] = fireball_row[i] + 1;
        else
          fireball_col[i] = 0; /* make it inactive */

        unsigned int fireball_disp_col = 0;
        if  (fireball_col[i] > 0)
          fireball_disp_col = fireball_col[i] - current_display_col;
          
        if ((fireball_disp_col == 0) || (fireball_disp_col >= 21))
          fireball_col[i] = 0;
        else
        {
          unsigned char fireball_disp_color = DISP_COLOR_RED;
          if ((game_count % 4) < 2)
            fireball_disp_color = DISP_COLOR_WHITE;
          /* display fireball */
          bigDispBoard[fireball_row[i]][fireball_disp_col] = fireball_disp_color;
        }
      }
    }
}

float mar_star_row = 0.0;
float mar_star_col = 0.0;
float mar_star_vert_speed = 0.0;
bool mar_star_dir_is_right = true;
unsigned int mar_star_count = 0;

void set_star(unsigned char input_row, unsigned int input_col)
{
  mar_star_row = input_row - 3.0;
  mar_star_col = input_col;
  mar_star_vert_speed = 0.0;
  mar_star_dir_is_right = true;
}

void display_mario_star(int current_mario_row, int current_mario_col, int current_display_col)
{
  if (mar_star_col > 0.0) /* active star */
  {
    if(mar_star_row < 22.0) /* star not in pit */
    {
      /* Update star vertical speed */
      mar_star_vert_speed = mar_star_vert_speed - HERO_ACCELERATION;
      if (mar_star_vert_speed < -2.0)
        mar_star_vert_speed = -2.0;

      if (on_solid_ground((int)mar_star_row, (int)mar_star_col))
        mar_star_vert_speed = 2.0;

      /* Update star row position */
      mar_star_row = mar_star_row - mar_star_vert_speed / 6;

      /* Update star col position */
      if (can_go_dir_mariobros(mar_star_dir_is_right, false, mar_star_row, mar_star_col, current_display_col) == false)
        mar_star_dir_is_right = !mar_star_dir_is_right;
      if (mar_star_dir_is_right)
        mar_star_col = mar_star_col + 2.0 / 6;
      else
        mar_star_col = mar_star_col - 2.0 / 6;

      /* Display star */
      int star_disp_col = (int)mar_star_col - current_display_col;
      int star_disp_row = (int)mar_star_row;

      if ((star_disp_col == 0) || (star_disp_col == 20) || (star_disp_row == 21))
        mar_star_col = 0.0;
      else
      {
        unsigned char star_color_1 = DISP_COLOR_RED;
        unsigned char star_color_2 = DISP_COLOR_WHITE;
        if ((game_count % 3) == 1)
        {
          star_color_1 = DISP_COLOR_WHITE;
          star_color_2 = DISP_COLOR_YELLOW;
        }
        else if ((game_count % 3) == 2)
        {
          star_color_1 = DISP_COLOR_YELLOW;
          star_color_2 = DISP_COLOR_RED;
        }
 
        bigDispBoard[star_disp_row][star_disp_col] = star_color_1;
        bigDispBoard[star_disp_row][star_disp_col + 1] = star_color_2;
        bigDispBoard[star_disp_row - 1][star_disp_col] = star_color_2;
        bigDispBoard[star_disp_row - 1][star_disp_col + 1] = star_color_1;
      }

      /* Check Mario eat star */
      if (((current_mario_row == (int)mar_star_row) || ((current_mario_row - 1) == (int)mar_star_row) || ((current_mario_row + 1) == (int)mar_star_row)) && 
        ((current_mario_col == (int)mar_star_col) || ((current_mario_col + 1) == (int)mar_star_col) || ((current_mario_col - 1) == (int)mar_star_col)) )
      {
        mar_star_col = 0.0; /* star is gone */
        mar_star_count = game_count; /* eaten */
        mario_is_star = true;
        total_score++;
        delay(600);
      }

    }
      
  } /* star is active */

  if ((mar_star_count > 0) && (game_count - mar_star_count < 200))
    mario_is_star = true;
  else
    mario_is_star = false;

}

/* Updates current_hero_speed (used by update_hero_vert_speed, update_hero_location, 
 *         hero_face_right (used by disp_mario), hero_is_walking (used by disp_mario)
 * Uses hero_run_count internally
 * Reads button presses to determine and apply effects of accelearation (updates current_hero_speed)  
 */
void update_hero_dir_speed(unsigned char move_dir, unsigned char button_press, int current_hero_row, int current_hero_col, int current_display_col, float * current_hero_speed)
{
  float hero_accel = HERO_ACCELERATION * 2; /* how quickly hero accelerates */
  bool button_right = ((move_dir & MOVE_RIGHT) > 0);
  bool button_left = ((move_dir & MOVE_LEFT) > 0);
  bool hero_is_running = (button_press & MOVE_ROTATE_LEFT) > 0; /* B Button pressed */
  hero_is_trying = button_right || button_left;

  /* Accelerate Hero from 2->3 at half the time Hero goes from 1->2 */
  if (hero_is_running) // B Button, Run
    hero_accel = hero_accel * 2;

  /* Update Speed */
  if ((button_right || (*current_hero_speed > 0))  && (hero_can_go_dir(true, mario_is_big, current_hero_row, current_hero_col, current_display_col) == false)) /* if can't go right, stop */
    *current_hero_speed = 0;
  else if ((button_left || (*current_hero_speed < 0))  && (hero_can_go_dir(false, mario_is_big, current_hero_row, current_hero_col, current_display_col) == false)) /* if can't go left, stop */
    *current_hero_speed = 0;
  else if ((current_hero_speed < 0) /* quickly decelerate to opposite direction */
      && (button_right))
    *current_hero_speed = apply_acceleration(*current_hero_speed, hero_accel * 2, hero_is_running);
  else if ((current_hero_speed > 0) /* quickly decelerate to opposite direction */
      && (button_left))
   *current_hero_speed = apply_acceleration(*current_hero_speed, -hero_accel * 2, hero_is_running);
  else if (button_right)
    *current_hero_speed = apply_acceleration(*current_hero_speed, hero_accel * 2, hero_is_running);
  else if (button_left)
    *current_hero_speed = apply_acceleration(*current_hero_speed, -hero_accel * 2, hero_is_running);
  else if (hero_jump_count == 0)
    *current_hero_speed = *current_hero_speed / 2;

  if (abs(*current_hero_speed) < 0.2)
    *current_hero_speed = 0.0;
  /* Update face direction */
  if (*current_hero_speed > 0)
    hero_face_right = true;
  else if (*current_hero_speed < 0)
    hero_face_right = false;

}




/* Determine if Mario can go up based on current_mario_row, applicable foreground items (current_mario_col)
 * and mario_is_big
 * Applicable items are bricks and ? blocks
 */
bool mario_can_go_up(int current_mario_row, int current_mario_col)
{
  /* Can't go up when hit blocks or ? */
  bool can_go_up = true;
  unsigned char low_row = 16;
  unsigned char high_row = 8;
  if (mario_is_big)
  {
    low_row = 18;
    high_row = 10;
  }

  /* Only look if we might actually hit something */
  if ((current_mario_row == low_row) || (current_mario_row == high_row))
  {
    bool left_is_low_q = ((pgm_read_word_near(&marioDispForeItems[current_mario_col]) & MARIO_LOW_Q) > 0);
    bool right_is_low_q = ((pgm_read_word_near(&marioDispForeItems[current_mario_col + 1]) & MARIO_LOW_Q) > 0);
    bool left_minus_one_is_low_q = ((pgm_read_word_near(&marioDispForeItems[current_mario_col - 1]) & MARIO_LOW_Q) > 0);
    bool right_plus_one_is_low_q = ((pgm_read_word_near(&marioDispForeItems[current_mario_col + 2]) & MARIO_LOW_Q) > 0);
    bool left_is_high_q = ((pgm_read_word_near(&marioDispForeItems[current_mario_col]) & MARIO_HIGH_Q) > 0);
    bool right_is_high_q = ((pgm_read_word_near(&marioDispForeItems[current_mario_col + 1]) & MARIO_HIGH_Q) > 0);
    bool left_minus_one_is_high_q = ((pgm_read_word_near(&marioDispForeItems[current_mario_col - 1]) & MARIO_HIGH_Q) > 0);
    bool right_plus_one_is_high_q = ((pgm_read_word_near(&marioDispForeItems[current_mario_col + 2]) & MARIO_HIGH_Q) > 0);
    unsigned int i;

    /* Check for Q */
    if ((current_mario_row == low_row) && (left_is_low_q || right_is_low_q))
    {
      can_go_up = false;
      unsigned int q_col = current_mario_col;
      if (left_is_low_q && left_minus_one_is_low_q)
        q_col--;
      else if (right_is_low_q && right_plus_one_is_low_q)
        q_col++;
      /* See if ? has been hit */
      bool found = false;
      for(i = 0; i < low_q_i; i++)
        if (locations_low_q[i] == q_col)
          found = true;

      if (found == false) /* haven't hit before */
      {
        locations_low_q[low_q_i] = q_col;
        low_q_i++;
        if ((q_col == low_mush_fire[0]) || (q_col == low_mush_fire[1]))
          set_mush_fire(14, q_col, false);
        else
          set_coin_animation(14, q_col);
      }
    }
    else if ((current_mario_row == low_row) && ((current_mario_col == MARIO_1UP_COL) || (current_mario_col + 1 == MARIO_1UP_COL) || (current_mario_col - 1 == MARIO_1UP_COL)))
    {
      can_go_up = false;
      if (mario_1up_hit == false)
      {
        locations_low_q[low_q_i] = MARIO_1UP_COL;
        low_q_i++;
        set_mush_fire(14, MARIO_1UP_COL, true);
        mario_1up_hit = true;
      }
    }
    else if ((current_mario_row == low_row) && ((current_mario_col == MARIO_REPEAT_BRICK_COL) || (current_mario_col + 1 == MARIO_REPEAT_BRICK_COL) || (current_mario_col - 1 == MARIO_REPEAT_BRICK_COL)))
    {
      can_go_up = false;
      if (mario_multi_brick_count == 0)
        mario_multi_brick_count = game_count;
      if ((game_count - mario_multi_brick_count) < 60)
      {
        set_coin_animation(14, MARIO_REPEAT_BRICK_COL);
        set_breaking_brick(14, MARIO_REPEAT_BRICK_COL, true);
      }
      else if (mario_multi_brick_count > 1)
      {
        locations_low_q[low_q_i] = MARIO_REPEAT_BRICK_COL;
        low_q_i++;
        mario_multi_brick_count = 1;
      }
      
    }
    else if ((current_mario_row == low_row) && ((current_mario_col == MARIO_STAR_COL) || (current_mario_col + 1 == MARIO_STAR_COL) || (current_mario_col - 1 == MARIO_STAR_COL)))
    {
      can_go_up = false;
      if (mario_star_hit == false)
      {
        locations_low_q[low_q_i] = MARIO_STAR_COL;
        low_q_i++;
        set_star(14, MARIO_STAR_COL);
        mario_star_hit = true;
      }
    }
    else if ((current_mario_row == high_row) && (left_is_high_q || right_is_high_q))
    {
      can_go_up = false;
      unsigned int q_col = current_mario_col;
      if (left_is_high_q && left_minus_one_is_high_q && (q_col != 276)) /* special case col */
        q_col--;
      else if (right_is_high_q && right_plus_one_is_high_q)
        q_col++;
      /* See if ? has been hit */
      bool found = false;
      for(i = 0; i < high_q_i; i++)
        if (locations_high_q[i] == q_col)
          found = true;

      if (found == false) /* haven't hit before */
      {
        locations_high_q[high_q_i] = q_col; /* only saves left column of ? */
        high_q_i++;
        if (q_col == high_mush_fire)
          set_mush_fire(6, q_col, false);
        else
          set_coin_animation(6, q_col);
      }
    }
    else
    {
      /* Check for bricks */
      for(i = 0; i < NUM_BRICKS; i++)
      {
        if (can_go_up == true)
        {
          if ((current_mario_row == low_row) && ((locations_low_bricks[i] == current_mario_col) || (locations_low_bricks[i] == current_mario_col + 1) || (locations_low_bricks[i] == current_mario_col - 1)))
          {
            can_go_up = false;            
            if (mario_is_big)
            {
              set_breaking_brick(current_mario_row - 4, locations_low_bricks[i], false);
              locations_low_bricks[i] = 22; /* Break brick */
            }
            else              
              set_breaking_brick(current_mario_row - 2, locations_low_bricks[i], true);
          }
          else if ((current_mario_row == high_row) && ((locations_high_bricks[i] == current_mario_col) || (locations_high_bricks[i] == current_mario_col + 1) | (locations_high_bricks[i] == current_mario_col - 1)))
          {
            can_go_up = false;              
            if (mario_is_big)
            {
              set_breaking_brick(current_mario_row - 4, locations_high_bricks[i], false);
              locations_high_bricks[i] = 22; /* Break brick */
            }
            else              
              set_breaking_brick(current_mario_row - 2, locations_high_bricks[i], true);
          }
        } /* broke brick (can go up is false) */
      }
    }

  }
  
  

  return can_go_up;
  
}


/* Determine if Gimkit can go up based on current_hero_row, current_hero_col, applicable foreground items.
 * Applicable items are platforms
 * 
 * Assume platforms are listed from lowest to highest
 * Platforms are identified by row, col of leftmost point => length
 */
bool gimkit_can_go_up(int current_hero_row, int current_hero_col)
{
  /* Can't go up when hit platforms*/
  bool can_go_up = true;

  /* Check for platforms */
  for(int i = 0; i < NUM_GIMKIT_PLATFORMS; i++)
  {
    unsigned char gimkit_head_row = current_hero_row - GIMKIT_HERO_HEIGHT + 1; // gimkit feet row is current_hero_row
    unsigned char gimkit_hero_far_col = current_hero_col + GIMKIT_HERO_WIDTH - 1; // gimkit near col is current_hero_col
    unsigned char platform_row = pgm_read_byte_near(&gkPlatform_Row[i]);
    unsigned char platform_col = pgm_read_byte_near(&gkPlatform_Col[i]);
    unsigned char platform_length = pgm_read_byte_near(&gkPlatform_Length[i]);
    unsigned char platform_far_col = platform_col + platform_length - 1;
    if (gimkit_head_row == platform_row + 1) // gimkit hero one row greater (one below) platform
    {
       if ((gimkit_hero_far_col >= platform_col) && (current_hero_col <= platform_far_col))
          can_go_up = false;
    }
//    else if (gimkit_head_row < platform_row) // gimkit hero at or above platform
//      break;
  }

  return can_go_up;
  
}

/* Determine if hero can go up based on current_hero_row, current_hero_col, applicable foreground items,
 * Applicable items are bricks and ? blocks
 */
bool hero_can_go_up(int current_hero_row, int current_hero_col)
{
  bool return_value = false;
  
  if (game_is_mario == true)
     return_value = mario_can_go_up(current_hero_row, current_hero_col);
  else
     return_value = gimkit_can_go_up(current_hero_row, current_hero_col);

  return return_value;
}

/* Updates current_hero_jump_speed (used by update_hero_location),
 *         hero_is_jumping (used by update_hero_dir_speed)
 * Uses hero_jump_count internally 
 * Reads button presses to determine and apply affects of vert accelearation (updates current_hero_jump_speed)  
 * 
 */

void update_hero_vert_speed(unsigned char button_press, int current_hero_row, int current_hero_col, float *current_hero_jump_speed, float current_hero_speed)
{
  
  float hero_accel = HERO_ACCELERATION * 4.0;
  bool hero_in_air = on_solid_ground(current_hero_row, current_hero_col) == false;
  bool hero_can_up = false;
  if (*current_hero_jump_speed > 0)
    hero_can_up = hero_can_go_up(current_hero_row, current_hero_col);

  /* Determine Jump Height */
  /* Hero jumps higher based on horizontal speed, how long press A button */
  if (abs(current_hero_speed) > 2.0)
    hero_jump_time_accel = 10;

  /* If land on ground, stop jump (look for pits)
   * If hit head, stop going up
   * If land on something, stop jump
   */
  /* Jumping has initiated and now back on ground or hit head */
  if ((hero_was_in_air == true) && (hero_in_air == false)) 
  {
    /* stop the jump */
    *current_hero_jump_speed = 0;
    hero_jump_count = 0;
    hero_jump_time_accel = NORMAL_JUMP_TIME;
  }
  if ((hero_can_up == false) && (*current_hero_jump_speed > 0))
  {
    /* stop the jump */
    *current_hero_jump_speed = -1;
  }
  else if ((((button_press & MOVE_ROTATE_RIGHT) > 0) && (hero_let_go_of_a)) && 
           (((hero_jump_count == 0) && (hero_in_air == false)) ||
           ((game_is_mario == false) && (num_hero_jumps == 1))))/* A Button, Jump */
  {
    hero_jump_count = game_count; /* Here Mario Jump Count is used to determine how long button is pressed */
    /* current_mario_jump_speed > 0 means moving up, < 0 is down */
    *current_hero_jump_speed = 3; /* Top Upward Jump Speed */
    hero_let_go_of_a = false;
    num_hero_jumps++;
  } /* on ground */

  if ((button_press & MOVE_ROTATE_RIGHT) == 0)
    hero_let_go_of_a = true;

  /* After initial jump, start downward effects of gravity */
  if ((hero_in_air) && (((game_count - hero_jump_count) >= hero_jump_time_accel) || (hero_jump_count == 0)))
    *current_hero_jump_speed = apply_acceleration(*current_hero_jump_speed, -hero_accel, true);

  hero_was_in_air = hero_in_air;

}

/* Updates hero horizontal location based on current_hero_speed, current_hero_col and 
 * current_display_col
 * Applies current_hero_speed to position 
 * (how many cycles it takes to update position based on current speed)
 * 
 *  current_hero_col
 *  Decrement without checking, assume fall will stop due to ground or die in hole
 *  Different top row max based on game
 *  
 *  (0 - 438 for Mario) 0 is first col, bottom left of hero  
 *  (0 - 255 for Gimkit) 0 is first col, bottom left of hero 
 *  
 *  game_is_mario - Mario or Gimkit
 * 
 */
void update_hero_hor_location(float current_hero_speed, int *current_hero_col, int *current_display_col)
{
  /* update rate based on current Mario speed */
  unsigned char update_rate = 0;
  int hero_speed = (int)current_hero_speed;

  unsigned int max_hor_cols = NUM_MARIO_COLUMNS - 12; // mario

  if (game_is_mario == false) // gimkit
     max_hor_cols = NUM_GIMKIT_COLS - GIMKIT_HERO_WIDTH;

  /* UPDATE HORIZONTAL POSITION */
  /* Only update if abs(speed) > 0 */
  if (hero_speed != 0)
  {
    if (((abs((int)hero_speed) == 1) && ((game_count % 2) == 0)) || 
        ((abs((int)hero_speed) == 2) && ((game_count % 3) < 2)) || 
        ((abs((int)hero_speed) == 3) && ((game_count % 4) < 3)))
    {
      /* Mario stays on left half of screen
       * If move right past midpoint -> move display right (current_display_col++)
       * If move left, can only move to left side of screen
       */
      if ((hero_speed > 0) && (*current_hero_col < max_hor_cols))
        (*current_hero_col)++; /* (0 - 438), bottom left of mario */

      if (game_is_mario == true)
      {
        if ((hero_speed < 0) && (*current_hero_col > *current_display_col))
          (*current_hero_col)--; /* (0 - 438), bottom left of mario */
        if ((*current_hero_col - *current_display_col) > 11) 
          (*current_display_col)++;
      }
      else // gimkit
      {
        if ((hero_speed < 0) && (*current_hero_col > 0))
          (*current_hero_col)--; /* (0 - 255), bottom left of hero */
        if (((*current_hero_col - *current_display_col) > 14) && (*current_display_col < (NUM_GIMKIT_COLS - NUM_DISP_ROWS - 1))) 
          (*current_display_col)++;
        else if (((*current_hero_col - *current_display_col) < 8) && (*current_display_col > 0)) 
          (*current_display_col)--;
      }
    }
  }
  
}


/* Updates hero vertical location based on current_hero_row, current_hero_jump_speed
 * Applies current_hero_jump_speed to position
 * (how many cycles it takes to update position based on current speed)
 *  
 *  current_hero_row
 *  Decrement without checking, assume fall will stop due to ground or die in hole
 *  Different top row max based on game
 *  
 *  (0 - 21 for Mario) 0 is top row, bottom left of hero  
 *  (0 - 250 for Gimkit) 0 is top row, bottom left of hero 
 *  
 *  current_display_row is top row of current display
 *  
 *  game_is_mario - Mario or Gimkit
 *  
  */
void update_hero_vert_location(float current_hero_jump_speed, int *current_hero_row, int *current_display_row)
{
  /* update rate based on current hero speed */
  unsigned char update_rate = 0;
  int hero_speed = (int)current_hero_jump_speed;

  unsigned int max_vert_rows = NUM_DISP_ROWS - 1; // mario

  if (game_is_mario == false) // gimkit
     max_vert_rows = NUM_GIMKIT_ROWS - 2;
  
  /* UPDATE VERTICAL POSITION */
  if (hero_speed != 0)
  {
    if (((abs(hero_speed) == 1) && ((game_count % 3) < 2)) || 
        ((abs(hero_speed) == 2) && ((game_count % 5) < 4)) || 
        (abs(hero_speed) == 3))
    {
      /* current_hero_jump_speed > 0 means moving up, < 0 is down */
      if ((hero_speed > 0) && (*current_hero_row > 0))
        (*current_hero_row)--; /* (0 - 21 for Mario) 0 is top row, bottom left of hero */
      else if ((hero_speed < 0) && (*current_hero_row < max_vert_rows))
        (*current_hero_row)++; /* (0 - 21 for Mario), bottom left of hero */

      // update display row
      if (game_is_mario == false) // only update display row for Gimkit
      {
        if ((*current_hero_row - *current_display_row) < 8) 
          (*current_display_row)--;
        else if (((*current_hero_row - *current_display_row) > 14)  && (*current_display_row < (NUM_GIMKIT_ROWS - NUM_DISP_ROWS)))
          (*current_display_row)++;
      }
    }
  }

}

void init_mario()
{
  hero_face_right = true;
  mario_is_big = false;
  mario_is_fire = false;
  hero_is_trying = false;
  mario_is_star = false;
  hero_jump_count = 0;
  coin_row = 0;
  coin_col = 0;
  coin_count = 0;
  mush_row = 0;
  mush_col = 0;
  mush_is_red = true;
  mush_go_right = true;
  game_is_mario = true;
  mush_count = 0;
  game_count = 0;
  
  mario_1up_hit = false;
  mario_star_hit = false;
  mario_multi_brick_count = 0;
  mario_bump_brick_count = 0;
  mush_is_flower = false;
  bumping_brick = false;
  mar_star_row = 0;
  mar_star_col = 0;
  mar_star_count = 0;
  goomba_col_done = 0;
  brick_bump_break_col == 0;
  brick_bump_break_count = 0;
  koopa_col = 0.0;
  koopa_moving = true;
  koopa_count = 0;
  koopa_kicked_right = false;
  koopa_kicked_left = false;
  mario_invulnerable_count = 0;
  goodbye_col = 0.0;

  /* Init breaking brick arrays */
  unsigned int i;
  for(i = 0; i < 4; i++)
  {
    breaking_brick_row[0][i] = 22.0;
    breaking_brick_row[1][i] = 22.0;
    breaking_brick_col[0][i] = 0.0;
    breaking_brick_col[1][i] = 0.0;
    breaking_brick_vert_speed[0][i] = 0.0;
    breaking_brick_vert_speed[1][i] = 0.0;
    
    goomba_row[i] = 0.0;
    goomba_col[i] = 0.0;
    goomba_face_right[i] = false;
  }

  /* Initialize ? Arrays */
  high_q_i = 0;
  low_q_i = 0;
  for(i = 0; i < NUM_Q; i++)
  {
    locations_low_q[i] = 0;
    if (i < NUM_HIGH_Q)
      locations_high_q[i] = 0;
  }

  /* Initialize brick arrays */
  unsigned char brick_col_h = 0;
  unsigned char brick_col_l = 0;
  for(i = 0; i < NUM_MARIO_COLUMNS; i = i + 2)
  {
    if ((pgm_read_word_near(&marioDispForeItems[i]) & MARIO_HIGH_BRICK) > 0)
    {
      locations_high_bricks[brick_col_h] = i;
      brick_col_h++;
    }
    if ((pgm_read_word_near(&marioDispForeItems[i]) & MARIO_LOW_BRICK) > 0)
    {
      locations_low_bricks[brick_col_l] = i;
      brick_col_l++;
    }
  }

}

#define MARIO_COUNT_END 2000
unsigned int mario_end_row = 0;
unsigned int mario_end_col = 0;
unsigned char mario_end_color = PLAY_MARIO_RED;

unsigned int play_mario(unsigned char mario_color)
{
  bool mario_over = false;
  unsigned int move_dir = MOVE_NONE;

  /* Re-init globabls */
  int current_mario_row = NUM_DISP_ROWS - 2; /* bottom left of mario */
  int current_mario_col = 6; /* (0 - 438), bottom left of mario */
  int current_display_col = 0; /* leftmost column of display as level column */
  int current_display_row = 0;

  total_score = 0;
  mario_lives = 2;

  float current_mario_speed = 0; /* (-3,3) */
  float current_mario_jump_speed = 0; /* (-3,3) */

  delay(300);

  while (mario_over == false)
  {
    /* look for movements */
    move_dir = getMove();

    /* Pause */
    if (move_dir == MOVE_START)
    {
      delay(300);
      move_dir = MOVE_NONE;
      while (move_dir != MOVE_START)
        move_dir = getMove();
      delay(300);
    }

    update_hero_dir_speed(move_dir & 0xF, (move_dir >> 4) & 0xF, current_mario_row, current_mario_col, current_display_col, &current_mario_speed);
    update_hero_hor_location(current_mario_speed, &current_mario_col, &current_display_col);
    update_hero_vert_speed((move_dir >> 4) & 0xF, current_mario_row, current_mario_col, &current_mario_jump_speed, current_mario_speed);
    update_hero_vert_location(current_mario_jump_speed, &current_mario_row, &current_display_row);
    game_count++;
   /* Game Over based on timer or fall in hole */
    if ((game_count >= MARIO_COUNT_END) || (current_mario_row == (NUM_DISP_ROWS - 1)))
      mario_over = true;

    disp_mario_back(current_display_col, mario_color);
    display_mario_back_items(current_display_col);
    display_mario_fore_items(current_display_col);
    disp_mario(mario_color, current_mario_row, current_mario_col, current_display_col);
    if (display_goombas(current_mario_row, current_mario_col, current_display_col, current_mario_jump_speed)) /* Goomba got him */
      mario_over = true;
    if (display_koopa(current_mario_row, current_mario_col, current_display_col, &current_mario_jump_speed))
      mario_over = true;
    disp_breaking_brick(current_display_col);
    display_coin_animation(current_display_col);
    display_mush(current_display_col, current_mario_row, current_mario_col);
    display_mario_fireballs(current_display_col, current_mario_row, current_mario_col, (move_dir >> 4) & 0xF);
    display_mario_star(current_mario_row, current_mario_col, current_display_col);
    display_goodbye_char(false, current_display_col);
    displayLEDs(true, false);

    /* Check for restart */
    if (mario_over == true)
    {
      /* Display animation if not a hole */
      if (current_mario_row < 21)
      {
        set_goodbye_char(current_mario_row, current_mario_col, MARIO_GOODBYE_MARIO, MOVE_NONE);
        
        // save current display
        unsigned char save_bg_0[NUM_DISP_ROWS] = {0};
        unsigned char save_bg_1[NUM_DISP_ROWS] = {0};
        for (int i = 0; i < NUM_DISP_ROWS; i++)
        {
          save_bg_0[i] = bigDispBoard[i][current_mario_col - current_display_col];
          save_bg_1[i] = bigDispBoard[i][current_mario_col - current_display_col + 1];
        }
        save_bg_0[current_mario_row] = DISP_COLOR_LIGHT_BLUE;
        save_bg_1[current_mario_row] = DISP_COLOR_LIGHT_BLUE;
        save_bg_0[current_mario_row - 1] = DISP_COLOR_LIGHT_BLUE;
        save_bg_1[current_mario_row - 1] = DISP_COLOR_LIGHT_BLUE;
        
        while (goodbye_col > 0.5)
        {
          // write saved display
          for (int i = 0; i < NUM_DISP_ROWS; i++)
          {
            bigDispBoard[i][current_mario_col - current_display_col] = save_bg_0[i];
            bigDispBoard[i][current_mario_col - current_display_col + 1] = save_bg_1[i];
          }
          
          display_goodbye_char(mario_color, current_display_col);
          
          
          displayLEDs(true, false);
          delay(5);
        }
        // write saved display
        for (int i = 18; i < NUM_DISP_ROWS; i++)
        {
          bigDispBoard[i][current_mario_col - current_display_col] = save_bg_0[i];
          bigDispBoard[i][current_mario_col - current_display_col + 1] = save_bg_1[i];
        }
        displayLEDs(true, false);
        
      }
      if (mario_lives > 0)
      {
        mario_over = false;
        bool temp = mario_1up_hit;
        init_mario();
        if (temp)
          mario_1up_hit = true;
        current_mario_row = 20;
        unsigned int col_adjust = 0;
        if (current_mario_col >= MARIO_RESTART_COL)
          col_adjust = MARIO_RESTART_COL - 12;
        current_mario_col = col_adjust + 6;
        current_display_col = col_adjust;
        mario_lives--;
        if (total_score > 10)
          total_score = total_score - 10;
        delay(1000);
      }
    } /* mario over */

    if (current_mario_col >= MARIO_FLAG_POLE_COL)
    {
      mario_over = true;
      total_score = total_score + (MARIO_COUNT_END - game_count) / 10 + (20 - current_mario_row);
    }

    delay(5);
  }
  
  delay(500);
  mario_end_row = current_mario_row;
  mario_end_col = current_mario_col;
  mario_end_color = mario_color;

  return total_score;
}




void display_mario_end_animation()
{
  unsigned int current_mario_row = mario_end_row;
  unsigned int current_mario_col = mario_end_col;
  unsigned int current_display_col = current_mario_col - 11;
  if (current_mario_col >= MARIO_FLAG_POLE_COL)
  {
    current_mario_col = MARIO_FLAG_POLE_COL;
    bool mario_animation_over = false;
    game_count = 0;
    bool display_mario_now = true;
    hero_face_right = true;
    unsigned char firework_display_count = 0;

    while (mario_animation_over == false)
    {
      /* only move on count */
      if ((game_count % 2) == 0)
      {
        /* First, descend the flag pole */
        if ((current_mario_col == MARIO_FLAG_POLE_COL) && (current_mario_row < 18))
          current_mario_row++;
        else if (current_mario_col == MARIO_FLAG_POLE_COL) /* then flip around */
        {
          current_mario_col++;
          current_display_col++;
          hero_face_right = false;
        }
        else if ((current_mario_col == (MARIO_FLAG_POLE_COL + 1)) && (current_mario_row < 20)) /* then jump down */
          current_mario_row++;
        else if (current_mario_col < 426) /* then walk over */
        {
          hero_face_right = true;
          current_mario_col++;
          current_display_col++;
        }
        else /* then disappear */
           display_mario_now = false;
      }

      disp_mario_back(current_display_col, mario_end_color);
      display_mario_back_items(current_display_col);
      display_mario_fore_items(current_display_col);
      if (display_mario_now)
        disp_mario(mario_end_color, current_mario_row, current_mario_col, current_display_col);
      displayLEDs(true, false);

      game_count++;
      delay(5);

      /* then fireworks */
      if (display_mario_now == false)
        mario_animation_over = true;
    }

    current_mario_row = 5;
    current_display_col = 8;
    while (firework_display_count < 18)
    {
      if (firework_display_count > 14)
      {
        current_mario_row = 7;
        current_display_col = 5;
      }
      else if (firework_display_count > 11)
      {
        current_mario_row = 2;
        current_display_col = 11;
      }
      else if (firework_display_count > 8)
      {
        current_mario_row = 10;
        current_display_col = 16;
      }
      else if (firework_display_count > 5)
      {
        current_mario_row = 5;
        current_display_col = 15;
      }
      else if (firework_display_count > 2)
      {
        current_mario_row = 9;
        current_display_col = 6;
      }

      if ((firework_display_count % 3) == 0)
      {
        bigDispBoard[current_mario_row][current_display_col] = DISP_COLOR_RED;
        bigDispBoard[current_mario_row - 1][current_display_col] = DISP_COLOR_RED;
        bigDispBoard[current_mario_row][current_display_col + 1] = DISP_COLOR_RED;
        bigDispBoard[current_mario_row - 1][current_display_col + 1] = DISP_COLOR_RED;
      }
      else if ((firework_display_count % 3) == 1)
      {
        bigDispBoard[current_mario_row][current_display_col] = DISP_COLOR_WHITE;
        bigDispBoard[current_mario_row - 1][current_display_col] = DISP_COLOR_WHITE;
        bigDispBoard[current_mario_row][current_display_col + 1] = DISP_COLOR_WHITE;
        bigDispBoard[current_mario_row - 1][current_display_col + 1] = DISP_COLOR_WHITE;
      }
      else if ((firework_display_count % 3) == 2)
      {
        bigDispBoard[current_mario_row][current_display_col] = DISP_COLOR_LIGHT_BLUE;
        bigDispBoard[current_mario_row - 1][current_display_col] = DISP_COLOR_LIGHT_BLUE;
        bigDispBoard[current_mario_row][current_display_col + 1] = DISP_COLOR_LIGHT_BLUE;
        bigDispBoard[current_mario_row - 1][current_display_col + 1] = DISP_COLOR_LIGHT_BLUE;
      }
      
      displayLEDs(true, false);

      firework_display_count++;
      delay(200);
    }
  }
}


























/* GIMKIT */

unsigned char gimkitColors[3] = {DISP_COLOR_AQUA, DISP_COLOR_PURPLE, DISP_COLOR_PINK};
#define CLOUD_SPACING 26


/* Game Menu Display */
const unsigned char PROGMEM gimkitMenuDisp[NUM_DISP_ROWS_MENU - 9][NUM_DISP_COLS_MENU] =
  {{5,5,5,5,0,4,0,5,0,0,0,5},
   {5,0,0,0,0,4,0,5,5,0,5,5},
   {5,0,5,5,0,4,0,5,0,5,0,5},
   {5,0,0,5,0,4,0,5,0,0,0,5},
   {5,5,5,5,0,4,0,5,0,0,0,5},
   {0,0,0,0,0,0,0,0,0,0,0,0},
   {5,0,0,5,0,4,0,5,5,5,5,5},
   {5,0,5,0,0,4,0,0,0,5,0,0},
   {5,5,0,0,0,4,0,0,0,5,0,0},
   {5,0,5,0,0,4,0,0,0,5,0,0},
   {5,0,0,5,0,4,0,0,0,5,0,0}};

void displayGimkitMenu()
{
  const unsigned int unused_rows_top = (NUM_DISP_ROWS - NUM_DISP_ROWS_MENU) / 2;
  const unsigned int unused_cols_left = (NUM_DISP_COLS - NUM_DISP_COLS_MENU) / 2;
  unsigned int move_dir_gim = MOVE_NONE;
  unsigned char current_level = 0;
  unsigned char current_color[3] = {PAINT_COLOR_AQUA, PAINT_COLOR_PURPLE, PAINT_COLOR_PINK};
  
  for(int i = 0; i < NUM_DISP_ROWS; i++)
    for(int j = 0; j < NUM_DISP_COLS; j++)
      bigDispBoard[i][j] = DISP_COLOR_BLACK;
    
  for(int i = 0; i < NUM_DISP_ROWS_MENU - 9; i++)
    for(int j = 0; j < NUM_DISP_COLS_MENU; j++)
      bigDispBoard[i + unused_rows_top][j + unused_cols_left] = pgm_read_byte_near(&gimkitMenuDisp[i][j]);

  while (move_dir_gim != MOVE_START)
  {

    /* look for movements */
    move_dir_gim = getMove();

    /* End game */
    if (move_dir_gim == MOVE_DOWN)
      current_level = (current_level + 1) % 3;
    else if (move_dir_gim == MOVE_UP)
      current_level = (current_level + 3 - 1) % 3;
    else if (move_dir_gim == MOVE_LEFT)
      current_color[current_level] = (current_color[current_level] + NUM_PAINT_COLORS - 1 ) % NUM_PAINT_COLORS;
    else if (move_dir_gim == MOVE_RIGHT)
      current_color[current_level] = (current_color[current_level] + 1) % NUM_PAINT_COLORS;

    for(int i = 0; i < 3; i++)
      gimkitColors[i] = pgm_read_byte_near(&paintColors[current_color[i]]);


    /* Display Hero */
    bigDispBoard[16][10] = pgm_read_byte_near(&paintColors[current_color[0]]); // top left pixel
    bigDispBoard[16][11] = pgm_read_byte_near(&paintColors[current_color[0]]); // top left pixel
    bigDispBoard[17][10] = pgm_read_byte_near(&paintColors[current_color[1]]); // top left pixel
    bigDispBoard[17][11] = pgm_read_byte_near(&paintColors[current_color[1]]); // top left pixel
    bigDispBoard[18][10] = pgm_read_byte_near(&paintColors[current_color[2]]); // top left pixel
    bigDispBoard[18][11] = pgm_read_byte_near(&paintColors[current_color[2]]); // top left pixel
  
    displayLEDs(true, false);

    delay(80);
  }

}


void display_gimkit_back(unsigned char current_display_row, unsigned char current_display_col)
{
  for(int i = 0; i < NUM_DISP_ROWS; i++)
  {
    for(int j = 0; j < NUM_DISP_COLS; j++)
    {
      // unsigned int back_item_word = pgm_read_word_near(&marioDispBackItems[current_display_col+j]);
    
      bigDispBoard[i][j] = DISP_COLOR_LIGHT_BLUE;
 
    }
  }

  // display big clouds
  // 5x1 bottom, 3x1 top
  // moves at half speed of foreground
  // starts at 3,3 or 9,16 or 14,7 on screen initially, moves opposite of user ( based on current display row, display col)
  // initial current_display_col = 0; initial current_display_row = NUM_GIMKIT_ROWS - NUM_DISP_ROWS;
  const unsigned char num_big_clouds = 3;
  unsigned char big_cloud_row[num_big_clouds] = {14, 3, 9};
  unsigned char big_cloud_col[num_big_clouds] = {7, 3, 16};
  unsigned char cloud_row = 0;
  unsigned char cloud_col = 0;

  for(int cloud = 0; cloud < num_big_clouds; cloud++)
  {
    cloud_row = (big_cloud_row[cloud] + CLOUD_SPACING - (current_display_row - (NUM_GIMKIT_ROWS - NUM_DISP_ROWS)) / 2) % CLOUD_SPACING; // 0 - (CLOUD_SPACING - 1)
    cloud_col = (big_cloud_col[cloud] + CLOUD_SPACING - current_display_col / 2) % CLOUD_SPACING;  // 0 - (CLOUD_SPACING - 1)
    for(int i = 0; i < 5; i++)
    {
      if ((cloud_row < NUM_DISP_ROWS) && (((cloud_col + i) % CLOUD_SPACING) < NUM_DISP_COLS))
         bigDispBoard[cloud_row][(cloud_col + i) % CLOUD_SPACING] = DISP_COLOR_HALF_WHITE;
      if ((i > 0) && (i < 4) && (((cloud_row + CLOUD_SPACING - 1) % CLOUD_SPACING) < NUM_DISP_ROWS) && (((cloud_col + i) % CLOUD_SPACING) < NUM_DISP_COLS))
         bigDispBoard[((cloud_row + CLOUD_SPACING - 1) % CLOUD_SPACING)][(cloud_col + i) % CLOUD_SPACING] = DISP_COLOR_HALF_WHITE;
    }
  }

  // display little clouds
  // 3x1 bottom, 1x1 top
  // moves at quarter speed of foreground
  // starts at 10,4 or 2,11 or 16,15 or 6,17 or 1,2 on screen initially, moves opposite of user ( based on current display row, display col)
  // initial current_display_col = 0; initial current_display_row = NUM_GIMKIT_ROWS - NUM_DISP_ROWS;

  const unsigned char num_little_clouds = 5;
  unsigned char little_cloud_row[num_little_clouds] = {10, 2, 16, 6, 1};
  unsigned char little_cloud_col[num_little_clouds] = {4, 11, 15, 17, 2};

  for(int cloud = 0; cloud < num_little_clouds; cloud++)
  {
    cloud_row = (little_cloud_row[cloud] + CLOUD_SPACING - (current_display_row - (NUM_GIMKIT_ROWS - NUM_DISP_ROWS)) / 4) % CLOUD_SPACING; // 0 - (CLOUD_SPACING - 1)
    cloud_col = (little_cloud_col[cloud] + CLOUD_SPACING - current_display_col / 4) % CLOUD_SPACING;  // 0 - (CLOUD_SPACING - 1)
    for(int i = 0; i < 3; i++)
    {
      if ((cloud_row < NUM_DISP_ROWS) && (((cloud_col + i) % CLOUD_SPACING) < NUM_DISP_COLS))
        bigDispBoard[cloud_row][(cloud_col + i) % CLOUD_SPACING] = DISP_COLOR_HALF_WHITE;
    }
    if ((((cloud_row + CLOUD_SPACING - 1) % CLOUD_SPACING) < NUM_DISP_ROWS) && (((cloud_col + 1) % CLOUD_SPACING) < NUM_DISP_COLS))
      bigDispBoard[((cloud_row + CLOUD_SPACING - 1) % CLOUD_SPACING)][(cloud_col + 1) % CLOUD_SPACING] = DISP_COLOR_HALF_WHITE;
  }
  
}







void disp_gimkit_platforms(unsigned char current_display_row, unsigned char current_display_col)
{
  /* Display Platforms */
  for(int i = 0; i < NUM_GIMKIT_PLATFORMS; i++)
  {
    unsigned char platRow = pgm_read_byte_near(&gkPlatform_Row[i]);
    // if platform row is within displayable rows
    if ((platRow >= current_display_row) && // platform is at or below top of screen
        (platRow <= current_display_row + NUM_DISP_ROWS)) // platform is at or above bottom of screen
    {
       unsigned char platCol = pgm_read_byte_near(&gkPlatform_Col[i]);
       unsigned char platLength = pgm_read_byte_near(&gkPlatform_Length[i]);
       unsigned char dispRow = platRow - current_display_row;
       for(int j = platCol; j < platCol + platLength; j++) // for each column in current platform
       {
          // if platform column within displayable columns
          if ((j >= current_display_col) && (j < current_display_col + NUM_DISP_COLS))
          {
             unsigned char dispCol = j - current_display_col;
             if (platRow != (current_display_row + NUM_DISP_ROWS)) // row not below displayable area
             {
                if (i <= gimkit_current_platform) // been there
                   bigDispBoard[dispRow][dispCol] = DISP_COLOR_HILL_GREEN;
                else
                   bigDispBoard[dispRow][dispCol] = DISP_COLOR_BROWN;
             }
             if ((platRow - 1) >= current_display_row) // row not above displayable area
             {
                if (i <= gimkit_current_platform) // been there
                   bigDispBoard[dispRow - 1][dispCol] = DISP_COLOR_GREEN;
                else
                   bigDispBoard[dispRow - 1][dispCol] = DISP_COLOR_RED;
             }
          }
       }
    }
    else if (platRow < current_display_row)// platform is above playable rows
      break;   
  }
}



/* Displays gimkit in current position based on current_hero_row, current_hero_col, current_display_row, current_display_col
 */
void disp_gimkit_hero(unsigned char current_hero_row, unsigned char current_hero_col, unsigned char current_display_row, unsigned char current_display_col)
{
  unsigned char hero_row_now = current_hero_row - current_display_row;
  unsigned char hero_col_now = current_hero_col - current_display_col;

  bool hero_is_jumping = hero_jump_count > 0;

  bigDispBoard[hero_row_now - 2][hero_col_now] = gimkitColors[0]; // top left pixel
  bigDispBoard[hero_row_now - 2][hero_col_now + 1] = gimkitColors[0];

  bigDispBoard[hero_row_now - 1][hero_col_now] = gimkitColors[1];
  bigDispBoard[hero_row_now - 1][hero_col_now + 1] = gimkitColors[1];

  if (hero_is_jumping == true)
  {
    if (current_hero_col > 0)
      bigDispBoard[hero_row_now][hero_col_now - 1] = gimkitColors[2]; // bottom left pixel
    if (current_hero_col < (NUM_GIMKIT_ROWS - GIMKIT_HERO_WIDTH))
      bigDispBoard[hero_row_now][hero_col_now + 2] = gimkitColors[2]; // bottom left pixel
  }
  else
  {
    bigDispBoard[hero_row_now][hero_col_now] = gimkitColors[2]; // bottom left pixel
    bigDispBoard[hero_row_now][hero_col_now + 1] = gimkitColors[2]; // bottom right pixel
  }

}


#define GIMKIT_COUNT_END 5000

unsigned int play_gimkit()
{
  bool gimkit_over = false;
  unsigned int move_dir = MOVE_NONE;

  /* Re-init globabls */
  int current_hero_row = NUM_GIMKIT_ROWS - 3; /* bottom left of hero */
  int current_hero_col = 6; /* (0 - 255), bottom left of hero */
  int current_display_col = 0; /* leftmost column of display as level column */
  int current_display_row = NUM_GIMKIT_ROWS - NUM_DISP_ROWS;

  hero_face_right = true;
  hero_is_trying = false;
  hero_jump_count = 0;
  game_is_mario = false;
  game_count = 0;
  total_score = 0;
  gimkit_current_platform = 0;
  num_hero_jumps = 0;

  float current_hero_speed = 0; /* (-3,3) */
  float current_hero_jump_speed = 0; /* (-3,3) */

  delay(300);

  displayGimkitMenu();

  delay(300);

  while (gimkit_over == false)
  {
    /* look for movements */
    move_dir = getMove();

    /* End game */
    if (move_dir == MOVE_SELECT)
      gimkit_over = true;

    update_hero_dir_speed(move_dir & 0xF, (move_dir >> 4) & 0xF, current_hero_row, current_hero_col, current_display_col, &current_hero_speed);
    update_hero_hor_location(current_hero_speed, &current_hero_col, &current_display_col);
    update_hero_vert_speed((move_dir >> 4) & 0xF, current_hero_row, current_hero_col, &current_hero_jump_speed, current_hero_speed);
    update_hero_vert_location(current_hero_jump_speed, &current_hero_row, &current_display_row);
    game_count++;
   /* Game Over based on timer or fall in hole */
    if ((game_count >= GIMKIT_COUNT_END) || (gimkit_current_platform == (NUM_GIMKIT_PLATFORMS - 1)))
      gimkit_over = true;

    display_gimkit_back(current_display_row, current_display_col);
    disp_gimkit_platforms(current_display_row, current_display_col);
    disp_gimkit_hero(current_hero_row, current_hero_col, current_display_row, current_display_col);
    
    displayLEDs(true, false);

    delay(5);
  }
  
  delay(500);

  return gimkit_current_platform;
}















/* PAC-MAN Functions */

#define NUM_PAC_LIVES_START 3
unsigned char num_pac_lives = NUM_PAC_LIVES_START;

#define NUM_GHOSTS 3

#define GHOST_HOUSE_ROW 10
#define GHOST_HOUSE_COL 10

#define PAC_START_ROW 19
#define PAC_START_COL 11

// Assume 22 rows, 22 columns
// Assume start top left
unsigned char current_pac_row = PAC_START_ROW;
unsigned char current_pac_col = PAC_START_COL;
unsigned char current_ghost_row[NUM_GHOSTS] = {GHOST_HOUSE_ROW, GHOST_HOUSE_ROW, GHOST_HOUSE_ROW};
unsigned char current_ghost_col[NUM_GHOSTS] = {GHOST_HOUSE_COL - 3, GHOST_HOUSE_COL, GHOST_HOUSE_COL + 3};
unsigned char previous_pac_row = PAC_START_ROW;
unsigned char previous_pac_col = PAC_START_COL;
unsigned char previous_ghost_row[NUM_GHOSTS] = {GHOST_HOUSE_ROW, GHOST_HOUSE_ROW, GHOST_HOUSE_ROW};
unsigned char previous_ghost_col[NUM_GHOSTS] = {GHOST_HOUSE_COL - 3, GHOST_HOUSE_COL, GHOST_HOUSE_COL + 3};

#define GHOST_MODE_CHASE    0
#define GHOST_MODE_SCATTER  1
#define GHOST_MODE_FRIGHT   2


/* ------------------------------------
 * | Mode    | Level 1 - 4 | Level 5+ | 
 * ------------------------------------
 * | Scatter |      7      |    5     |
 * ------------------------------------ 
 * | Chase   |     20      |   20     |
 * ------------------------------------
 */

/* Speeds are implemented using combination of delay timer and ghost / pac man update frequency
 * As a result, speeds will not exactly match true Pac-Man implementation
 */
/*  -------------------------------------------------------------------
 * |       |              PAC MAN SPEED              |  GHOSTS SPEED |
 * -------------------------------------------------------------------
 * | LEVEL | NORM | NORM DOTS | FRIGHT | FRIGHT DOTS | NORM | FRIGHT | 
 * -------------------------------------------------------------------
 * | 1 - 4 |   80 |      70   |   100  |      80     |  75  |   50   |
 * -------------------------------------------------------------------
 * |   5   |   90 |      80   |   100  |      90     |  95  |   60   |
 * -------------------------------------------------------------------
 */
/* 
 * Table below is what percentage of cycles needed to show that speed
 * delay 10 * (mod 5) = nominal 20 Hz - set as speed 80
 * 50 = 12.5 Hz -> (mod 8) == 0
 * 60 = 15 Hz -> (mod 7) == 0
 * 70 = 17.5 Hz -> (mod 6) == 0 || (mod 100) == 1
 * 80 = 20 Hz -> (mod 5) == 0
 * 90 = 22.5 Hz ((mod 5) == 0 || (mod 40) == 1 
 * 100 = 25 Hz -> (mod 4) == 0
 * -------------------------------------------------------------------
 * |       |              PAC MAN SPEED              |  GHOSTS SPEED |
 * -------------------------------------------------------------------
 * | LEVEL | NORM | NORM DOTS | FRIGHT | FRIGHT DOTS | NORM | FRIGHT | 
 * -------------------------------------------------------------------
 * | 1 - 4 |   80 |      70   |   100  |      80     |  70  |   50   |
 * -------------------------------------------------------------------
 * |   5   |   90 |      80   |   100  |      90     |  90  |   60   | 
 * -------------------------------------------------------------------
 */

/* just used to constrain speed options */
#define PAC_SPEED_50 50
#define PAC_SPEED_60 60
#define PAC_SPEED_70 70
#define PAC_SPEED_80 80
#define PAC_SPEED_90 90
#define PAC_SPEED_100 100

/* Current Pac Level (1-20) */
#define NUM_PAC_LEVELS 20
unsigned int current_pac_level = 0;


// dots on rows 3, 6, 9, 12, 15, 18 (starting at 0, skipping top unused row in 22x22)
// dots on cols 2, 5, 8, 11, 14, 17, 20 (starting at 0)
// bits go left to right, start top left (LSB is left)
#define NUM_DOTS_ROWS 6
#define NUM_DOTS_COLS 7
/* Currently active dots in maze */
unsigned int current_dots[NUM_DOTS_ROWS];


/* Timer Counter for current level of Pac Man */
unsigned int pac_counter = 0;
/* Timer Count for when last big dot was eaten */
unsigned int big_dot_eaten_counter = 0;
/* Timer Count for when last dot was eaten */
unsigned int dot_eaten_counter = 0;

#define DOT_TIME_EFFECT_PAC 15
#define BIG_DOT_TIME_EFFECT_GHOST 40

#define MOVE_LEFT_MASK 0x1
#define MOVE_RIGHT_MASK 0x2
#define MOVE_UP_MASK 0x4
#define MOVE_DOWN_MASK 0x8

unsigned char pac_current_dir = MOVE_NONE;

unsigned char ghost_current_dir[NUM_GHOSTS] = {MOVE_RIGHT, MOVE_UP, MOVE_LEFT};

/* Bool Timers used to determine when to move Pac */
bool timer_50 = false; // 12.5 Hz -> (mod 8) == 0
bool timer_60 = false; // 15 Hz -> (mod 7) == 0
bool timer_70 = false; // 17.5 Hz -> (mod 6) == 0 || (mod 100) == 1
bool timer_80 = false; // 20 Hz -> (mod 5) == 0
bool timer_90 = false; // 22.5 Hz ((mod 5) == 0 || (mod 40) == 19 
bool timer_100 = false; // 25 Hz -> (mod 4) == 0

bool ghost_face_right[NUM_GHOSTS] = {true, true, true};

/* Ghost Mode last cycle ghosts moved */
unsigned char previous_ghost_mode = GHOST_MODE_CHASE;


/* Pac Man Start Screen Display */
const unsigned char PROGMEM pacStartDisp[11][NUM_DISP_COLS_TETRIS] =
  {{6,6,6,4,4,4,0,8,8,0},
   {6,0,6,4,0,4,8,0,0,0},
   {6,6,6,4,4,4,8,0,0,0},
   {6,0,0,4,0,4,8,0,0,0},
   {6,0,0,4,0,4,0,8,8,0},
   {0,0,0,0,0,0,0,0,0,0},
   {3,0,3,5,5,5,1,1,1,0},
   {3,3,3,5,0,5,1,0,1,0},
   {3,0,3,5,5,5,1,0,1,0},
   {3,0,3,5,0,5,1,0,1,0},
   {3,0,3,5,0,5,1,0,1,0}};

void displayPacStart()
{
  /* Use Tetris Playfield to display Pac Man Start screen */
  unsigned int unused_rows_top = (NUM_DISP_ROWS - NUM_DISP_ROWS_TETRIS) / 2;
  unsigned int unused_cols_left = (NUM_DISP_COLS - NUM_DISP_COLS_TETRIS) / 2;
  
  /* PAC MAN Splash Screen */
  /* Use Tetris Playfield to display Pac Man Start screen */
  unsigned int i, j;
  for(i = 0; i < NUM_DISP_ROWS; i++)
    for(j = 0; j < NUM_DISP_COLS; j++)
      bigDispBoard[i][j] = DISP_COLOR_BLACK;
    
  for(i = 0; i < 11; i++)
    for(j = 0; j < NUM_DISP_COLS_TETRIS; j++)
      bigDispBoard[i + unused_rows_top][j + unused_cols_left] = pgm_read_byte_near(&pacStartDisp[i][j]);

  displayLEDs(true, false);
}

const unsigned char PROGMEM pac_back[NUM_DISP_ROWS][NUM_DISP_COLS] =
  {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 0
   {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}, // 1
   {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, // 2
   {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, // 3
   {1,0,0,1,0,0,1,1,1,1,1,1,1,1,1,1,0,0,1,0,0,1}, // 4
   {1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1}, // 5
   {1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1}, // 6
   {1,0,0,1,0,0,1,1,1,1,0,0,1,1,1,1,0,0,1,0,0,1}, // 7
   {1,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,1}, // 8
   {1,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,1}, // 9
   {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0}, // 10
   {0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0}, // 11
   {1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1}, // 12
   {1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1}, // 13
   {1,0,0,1,0,0,1,1,1,1,0,0,1,1,1,1,0,0,1,0,0,1}, // 14
   {1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1}, // 15
   {1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1}, // 16
   {1,0,0,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,0,0,1}, // 17
   {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, // 18
   {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, // 19
   {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}, // 20
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};// 21

/* Display the background as defined in pac_back */
void init_pac_display()
{
  unsigned int i, j;
  
  /* bottom row to top, right to left */
  for(i = 0; i < NUM_DISP_ROWS; i++)
  {
    for(j = 0; j < NUM_DISP_COLS; j++)
    {      
      if(pgm_read_byte_near(&pac_back[i][j]))//(pac_back[i][j] > 0)
        bigDispBoard[i][j] = DISP_COLOR_HALF_BLUE;
      else
        bigDispBoard[i][j] = DISP_COLOR_BLACK;
    }
  }

}


/* Clear previous positions of ghosts, pac */
/* Need to call this before displaying ghosts, pac, dots */
void clear_ghosts_pac()
{
  unsigned int i = 0;
  /* Clear previous positions */ 
  bigDispBoard[previous_pac_row - 1][previous_pac_col] = DISP_COLOR_BLACK; // top left pixel
  bigDispBoard[previous_pac_row - 1][(previous_pac_col + 1) % NUM_DISP_COLS] = DISP_COLOR_BLACK; // top right pixel
  bigDispBoard[previous_pac_row][previous_pac_col] = DISP_COLOR_BLACK; // bottom left pixel
  bigDispBoard[previous_pac_row][(previous_pac_col + 1) % NUM_DISP_COLS] = DISP_COLOR_BLACK; // bottom right pixel

  for (i = 0; i < NUM_GHOSTS; i++)
  {
    bigDispBoard[previous_ghost_row[i] - 1][previous_ghost_col[i]] = DISP_COLOR_BLACK; // top left pixel
    bigDispBoard[previous_ghost_row[i] - 1][(previous_ghost_col[i] + 1) % NUM_DISP_COLS] = DISP_COLOR_BLACK; // top right pixel
    bigDispBoard[previous_ghost_row[i]][previous_ghost_col[i]] = DISP_COLOR_BLACK; // bottom left pixel
    bigDispBoard[previous_ghost_row[i]][(previous_ghost_col[i] + 1) % NUM_DISP_COLS] = DISP_COLOR_BLACK; // bottom right pixel
  }
}

/* Display Pac Man on bigDispBoard */
/* Blink Pac's eye when moving (display one pixel as black, tried actual blinking but didn't look as good) */
void display_pac_man()
{
  
  //Display Pac Man
  bigDispBoard[current_pac_row - 1][current_pac_col] = DISP_COLOR_YELLOW; // top left pixel
  bigDispBoard[current_pac_row ][current_pac_col] = DISP_COLOR_YELLOW; // bottom left pixel
  bigDispBoard[current_pac_row - 1][(current_pac_col + 1) % NUM_DISP_COLS] = DISP_COLOR_YELLOW; // top right pixel
  bigDispBoard[current_pac_row][(current_pac_col + 1) % NUM_DISP_COLS] = DISP_COLOR_YELLOW; // bottom right pixel
  
  if (previous_pac_col < current_pac_col) // moved right
    bigDispBoard[current_pac_row - 1][(current_pac_col + 1) % NUM_DISP_COLS] = DISP_COLOR_BLACK; // top right pixel
  else if (previous_pac_col > current_pac_col) // moved left
    bigDispBoard[current_pac_row - 1][current_pac_col] = DISP_COLOR_BLACK; // top left pixel
  else if (previous_pac_row > current_pac_row) // moved up
    bigDispBoard[current_pac_row - 1][(current_pac_col + 1) % NUM_DISP_COLS] = DISP_COLOR_BLACK; // top right pixel
  else if (previous_pac_row < current_pac_row) // moved down
    bigDispBoard[current_pac_row][(current_pac_col + 1) % NUM_DISP_COLS] = DISP_COLOR_BLACK; // bottom right pixel

}



/* Display the three ghosts on to bigDispBoard */
/* Ghosts display differently based on direction facing, current ghost mode */
void display_ghosts()
{
  unsigned int i;
  unsigned char bottom_color[3] = {DISP_COLOR_PAC_BLUE,DISP_COLOR_PAC_BLUE,DISP_COLOR_PAC_BLUE};
  unsigned char back_top_color = DISP_COLOR_PAC_BLUE;
  unsigned char front_top_color = DISP_COLOR_HALF_WHITE;
  
  if (get_ghost_mode() != GHOST_MODE_FRIGHT)
  {
    back_top_color = DISP_COLOR_HALF_WHITE;
    front_top_color = DISP_COLOR_HALF_BLUE;
    bottom_color[0] = DISP_COLOR_HALF_RED;
    bottom_color[1] = DISP_COLOR_PINK;
    bottom_color[2] = DISP_COLOR_AQUA;
  }

  for (i ==0; i < NUM_GHOSTS; i++)
  {
    if (previous_ghost_col[i] < current_ghost_col[i]) // direction that was moved
      ghost_face_right[i] = true;
    else if (previous_ghost_col[i] > current_ghost_col[i]) // direction that was moved
      ghost_face_right[i] = false;

    if (ghost_face_right[i] == true)
    {
      bigDispBoard[current_ghost_row[i] - 1][current_ghost_col[i]] = back_top_color; // top left pixel
      bigDispBoard[current_ghost_row[i] - 1][(current_ghost_col[i] + 1) % NUM_DISP_COLS] = front_top_color; // top right pixel
    }
    else
    {
      bigDispBoard[current_ghost_row[i] - 1][current_ghost_col[i]] = front_top_color; // top left pixel
      bigDispBoard[current_ghost_row[i] - 1][(current_ghost_col[i] + 1) % NUM_DISP_COLS] = back_top_color; // top right pixel
    }
    bigDispBoard[current_ghost_row[i]][current_ghost_col[i]] = bottom_color[i]; // bottom left pixel
    bigDispBoard[current_ghost_row[i]][(current_ghost_col[i] + 1) % NUM_DISP_COLS] = bottom_color[i]; // bottom right pixel
  }

}


/* Display the current dots in current_dots on to bigDispBoard */
void display_dots()
{
  unsigned int i,j;
  unsigned char dot_color = DISP_COLOR_PAC_DOT;

  for (i = 0; i < NUM_DOTS_ROWS; i++)
  {
    for (j = 0; j < NUM_DOTS_COLS; j++)
    {
      if (((i == 0) || (i == NUM_DOTS_ROWS - 1)) && ((j == 0) || (j == NUM_DOTS_COLS - 1)))
        dot_color = DISP_COLOR_PURPLE;
      else
        dot_color = DISP_COLOR_PAC_DOT;
      if ((current_dots[i] & (1 << j)) > 0)
        bigDispBoard[3 * i + 3][3 * j + 2] = dot_color;
      else
        bigDispBoard[3 * i + 3][3 * j + 2] = DISP_COLOR_BLACK;
    }
  }

}

/* Display the current Pac lives on to bigDispBoard */
void display_pac_lives()
{
  if (num_pac_lives > 2)
    bigDispBoard[8][8] = DISP_COLOR_YELLOW;
  if (num_pac_lives > 1)
    bigDispBoard[8][7] = DISP_COLOR_YELLOW;
}



/* Get current ghost mode, same for all ghosts */
unsigned char get_ghost_mode()
{
  unsigned char ghost_mode;
  /* Determine Ghost Mode */
  if ((pac_counter % 100) < 20)
    ghost_mode = GHOST_MODE_SCATTER;
  else
    ghost_mode = GHOST_MODE_CHASE;
  if ((pac_counter - big_dot_eaten_counter) <= BIG_DOT_TIME_EFFECT_GHOST)
    ghost_mode = GHOST_MODE_FRIGHT;
    
  return ghost_mode;
}

/* Get current ghost speed, same for all ghosts */
unsigned char get_ghost_speed()
{
  unsigned char ghost_speed = PAC_SPEED_50;
  unsigned char ghost_mode = get_ghost_mode();
  if (current_pac_level < 5)
  {
      if (ghost_mode == GHOST_MODE_FRIGHT)
        ghost_speed = PAC_SPEED_50;
      else
        ghost_speed = PAC_SPEED_70;
  }
  else 
  {
    if (ghost_mode == GHOST_MODE_FRIGHT)
        ghost_speed = PAC_SPEED_60;
      else
        ghost_speed = PAC_SPEED_90;
  }
  return ghost_speed;
}



/* Attempt to move Ghost 1 (Blinky, the Red One), Ghost 2 (Pinky, the Pink One), Ghost 3 (Inky, the Light Blue One)  */
/* Only move when appropriate timer goes off based on current speed */
/* Current speed based on current mode, level */
/* Current behavior based on current mode */
/* In Chase, Blinky tries to go where Pac Man is, Pinky tries to go 4 tiles ahead of where Pac Man is */
/* Blinky starts moving immediately, Pinky and Inky start moving after set periods of time */
void move_ghost(unsigned char ghost_num, unsigned char ghost_move_options)
{
  unsigned char rand_direction_mask;
  bool move_found = false;
  unsigned char desired_direction = MOVE_NONE;
  unsigned char ghost_mode = get_ghost_mode();
  bool can_move_right = ((ghost_move_options & MOVE_RIGHT_MASK) > 0) && (current_ghost_col[ghost_num] < NUM_DISP_COLS - 2);
  bool can_move_left = ((ghost_move_options & MOVE_LEFT_MASK) > 0) && (current_ghost_col[ghost_num] > 0);
  bool can_move_down = ((ghost_move_options & MOVE_DOWN_MASK) > 0);
  bool can_move_up = ((ghost_move_options & MOVE_UP_MASK) > 0);

  unsigned char projected_row = current_pac_row;
  unsigned char projected_col = current_pac_col;
  unsigned char goal_pac_row = current_pac_row;
  unsigned char goal_pac_col = current_pac_col;

  if (ghost_num == 2)
  {
    if (goal_pac_row < 4)
      projected_row = 4;
    if (goal_pac_col < 4)
      projected_col = 4;

    /* Update projected pac location */
    if (pac_current_dir == MOVE_UP)
      projected_row = goal_pac_row - 4;
    else if (pac_current_dir == MOVE_DOWN)
      projected_row = goal_pac_row + 4;
    else if (pac_current_dir == MOVE_RIGHT)
      projected_col = goal_pac_col + 4;
    else if (pac_current_dir == MOVE_LEFT)
      projected_col = goal_pac_col - 4;
  }
  else if (ghost_num == 3)
  {
    /* Update desired location Ghost 3 */
    if (pac_current_dir == MOVE_UP)
      projected_row = current_pac_row - 2;
    else if (pac_current_dir == MOVE_DOWN)
      projected_row = current_pac_row + 2;
    else if (pac_current_dir == MOVE_RIGHT)
      projected_col = current_pac_col + 2;
    else if (pac_current_dir == MOVE_LEFT)
      projected_col = current_pac_col - 2;
    projected_row = 2 * (projected_row - current_ghost_row[ghost_num]) + current_ghost_row[ghost_num];
    projected_col = 2 * (projected_col - current_ghost_col[ghost_num]) + current_ghost_col[ghost_num];
  }
  
  
  /* Update previous positions */
  previous_ghost_row[ghost_num] = current_ghost_row[ghost_num];
  previous_ghost_col[ghost_num] = current_ghost_col[ghost_num];

  /* Once moving in a direction, ghosts can't reverse direction unless there is a mode change */
  /* However, they can turn */
  if (previous_ghost_mode == ghost_mode)
  {
    if ((ghost_current_dir[ghost_num] == MOVE_RIGHT) && can_move_right)
      desired_direction = MOVE_RIGHT;
    else if ((ghost_current_dir[ghost_num] == MOVE_LEFT) && can_move_left)
      desired_direction = MOVE_LEFT;
    else if ((ghost_current_dir[ghost_num] == MOVE_UP) && can_move_up)
      desired_direction = MOVE_UP;
    else if ((ghost_current_dir[ghost_num] == MOVE_DOWN) && can_move_down)
      desired_direction = MOVE_DOWN;
  }
    
  if (ghost_mode == GHOST_MODE_CHASE)
  {
    if ((ghost_current_dir[ghost_num] == MOVE_RIGHT) || (ghost_current_dir[ghost_num] == MOVE_LEFT))
    {
      if ((projected_row < current_ghost_row[ghost_num]) && can_move_up)
        desired_direction = MOVE_UP;
      else if ((projected_row > current_ghost_row[ghost_num]) && can_move_down)
        desired_direction = MOVE_DOWN;
    }
    if ((ghost_current_dir[ghost_num] == MOVE_UP) || (ghost_current_dir[ghost_num] == MOVE_DOWN))
    {
      if ((projected_col > current_ghost_col[ghost_num]) && can_move_right)
        desired_direction = MOVE_RIGHT;
      else if ((projected_col < current_ghost_col[ghost_num]) && can_move_left)
        desired_direction = MOVE_LEFT;
    }
  }
  else if ((ghost_mode == GHOST_MODE_SCATTER) && (desired_direction == MOVE_NONE))
  {
    /* Move to the top right corner */
    if ((can_move_up) && ((ghost_num == 1) || (ghost_num == 2)))
      desired_direction = MOVE_UP;
    else if ((can_move_right) && ((ghost_num == 1) || (ghost_num == 3)))
      desired_direction = MOVE_RIGHT;
    else if (can_move_left)
      desired_direction = MOVE_LEFT;
    else if (can_move_down)
      desired_direction = MOVE_DOWN;
    else if (can_move_right)
      desired_direction = MOVE_RIGHT;
    else if (can_move_up)
      desired_direction == MOVE_UP;
  }
  else if (ghost_mode == GHOST_MODE_FRIGHT)
  {
    while (desired_direction == MOVE_NONE)
    {
      rand_direction_mask = 1 << random(4);
      if ((ghost_move_options & rand_direction_mask) > 0)
      {
        if ((rand_direction_mask == MOVE_RIGHT_MASK) && can_move_right)
          desired_direction = MOVE_RIGHT;
        else if ((rand_direction_mask == MOVE_LEFT_MASK) && can_move_left)
          desired_direction = MOVE_LEFT;
        else if (rand_direction_mask == MOVE_UP_MASK)
          desired_direction = MOVE_UP;
        else if (rand_direction_mask == MOVE_DOWN_MASK)
          desired_direction = MOVE_DOWN;
      } /* if can move in that direction */
    } /* while no new direction */
          
  } /* ghost mode FRIGHT */
  
  if (desired_direction == MOVE_NONE)
  {
    if (can_move_up)
      desired_direction = MOVE_UP;
    else if (can_move_down)
      desired_direction = MOVE_DOWN;
    else if (can_move_right)
      desired_direction = MOVE_RIGHT;
    else if (can_move_left)
      desired_direction = MOVE_LEFT;
  }

  ghost_current_dir[ghost_num] = desired_direction;
  
  /* Move the Ghost */
  if ((ghost_current_dir[ghost_num] == MOVE_DOWN) && can_move_down)
    current_ghost_row[ghost_num]++;
  else if ((ghost_current_dir[ghost_num] == MOVE_UP) && can_move_up)
    current_ghost_row[ghost_num]--;
  else if ((ghost_current_dir[ghost_num] == MOVE_RIGHT) && can_move_right)
    current_ghost_col[ghost_num]++;
  else if ((ghost_current_dir[ghost_num] == MOVE_LEFT) && can_move_left)
    current_ghost_col[ghost_num]--;
  
  
}





/* Get Current Pac Speed */
unsigned char get_pac_speed()
{
  bool dot_eaten = (pac_counter - big_dot_eaten_counter) <= DOT_TIME_EFFECT_PAC;
  unsigned char pac_speed = 0;
  unsigned char ghost_mode = get_ghost_mode();
 
  if (current_pac_level < 5)
  {
      if ((ghost_mode == GHOST_MODE_FRIGHT) && (dot_eaten == true))
        pac_speed = PAC_SPEED_80;
      else if (ghost_mode == GHOST_MODE_FRIGHT)
        pac_speed = PAC_SPEED_100;
      else if (dot_eaten == true)
        pac_speed = PAC_SPEED_70;
      else
        pac_speed = PAC_SPEED_80;
  }
  else 
  {
    if ((ghost_mode == GHOST_MODE_FRIGHT) && (dot_eaten == true))
        pac_speed = PAC_SPEED_90;
      else if (ghost_mode == GHOST_MODE_FRIGHT)
        pac_speed = PAC_SPEED_100;
      else if (dot_eaten == true)
        pac_speed = PAC_SPEED_80;
      else
        pac_speed = PAC_SPEED_90;
  }
  return pac_speed;
}

/* Called to attempt move of Pac Man */
/* Pac only moves on timer based on current speed */
/* Pre-loads movements to support cornering */
/* Pac continues in current direction until turn or wall */
bool move_pac_man(unsigned char loaded_move_dir, unsigned char pac_move_options)
{
  bool can_move_right = ((pac_move_options & MOVE_RIGHT_MASK) > 0);
  bool can_move_left = ((pac_move_options & MOVE_LEFT_MASK) > 0);
  bool can_move_down = ((pac_move_options & MOVE_DOWN_MASK) > 0);
  bool can_move_up = ((pac_move_options & MOVE_UP_MASK) > 0);
  unsigned char initial_move_direction = pac_current_dir;

  /* Update previous position for display purposes */
  previous_pac_row = current_pac_row;
  previous_pac_col = current_pac_col;

  /* See if Pac should change directions */
  if ((loaded_move_dir == MOVE_RIGHT) && can_move_right)
    pac_current_dir = MOVE_RIGHT;
  else if ((loaded_move_dir == MOVE_LEFT) && can_move_left)
    pac_current_dir = MOVE_LEFT;
  else if ((loaded_move_dir == MOVE_UP) && can_move_up)
    pac_current_dir = MOVE_UP;
  else if ((loaded_move_dir == MOVE_DOWN) && can_move_down)
    pac_current_dir = MOVE_DOWN;

  /* Move Pac */
  if ((pac_current_dir == MOVE_RIGHT) && can_move_right)
    current_pac_col = (current_pac_col + 1) % NUM_DISP_COLS;
  else if ((pac_current_dir == MOVE_LEFT) && can_move_left)
    current_pac_col = (current_pac_col + NUM_DISP_COLS - 1) % NUM_DISP_COLS;
  else if ((pac_current_dir == MOVE_UP) && can_move_up)
    current_pac_row--;
  else if ((pac_current_dir == MOVE_DOWN) && can_move_down)
    current_pac_row++;
  
  return (initial_move_direction != pac_current_dir); /* Changed Direction */
}

/* Looks at input speed and current counter to determine if it is time for the caller to move */
bool time_to_move(unsigned char input_speed)
{
  bool it_is_time_to_move = false;
  /* Bool Timers used to determine when to move Pac */
  /* Updated here, but used in individual ghost,pac move functions */
  timer_50 = (pac_counter % 3) == 1; // 12.5 Hz -> 100 (mod 8) == 0
  timer_60 = (pac_counter % 2) == 0; // 15 Hz -> 100 (mod 7) == 0
  timer_70 = (pac_counter % 3) < 2; // 17.5 Hz -> 100 (mod 6) == 0 || 100 (mod 100) == 1
  timer_80 = (pac_counter % 4) < 3; // 20 Hz -> 100 (mod 5) == 0
  timer_90 = (pac_counter % 5) < 4; // 22.5 Hz (100 (mod 5) == 0 || 100 (mod 40) == 19 
  timer_100 = (pac_counter % 6) < 5;//pac_counter % 2 == 0; // 25 Hz -> 100 (mod 4) == 0

  it_is_time_to_move =
       (((input_speed == PAC_SPEED_50) && (timer_50 == true)) ||
        ((input_speed == PAC_SPEED_60) && (timer_60 == true)) ||
        ((input_speed == PAC_SPEED_70) && (timer_70 == true)) ||
        ((input_speed == PAC_SPEED_80) && (timer_80 == true)) ||
        ((input_speed == PAC_SPEED_90) && (timer_90 == true)) ||
        ((input_speed == PAC_SPEED_100) && (timer_100 == true)));

  return it_is_time_to_move;
}

/* Returns true if Pac and Ghost occupy same spot */
/* Considers if same pixel location, or if one pixel off based on Pac Direction (one in front but not behind) */
bool occupy_same_spot(unsigned char ghost_num)
{
  bool same_spot = false;
  if (((current_pac_row == current_ghost_row[ghost_num]) && (current_pac_col == current_ghost_col[ghost_num])) ||
      ((pac_current_dir == MOVE_RIGHT) && (current_pac_row == current_ghost_row[ghost_num]) && (current_pac_col == (current_ghost_col[ghost_num] - 1))) ||
      ((pac_current_dir == MOVE_LEFT) && (current_pac_row == current_ghost_row[ghost_num]) && (current_pac_col == (current_ghost_col[ghost_num] + 1))) ||
      ((pac_current_dir == MOVE_UP) && (current_pac_row == (current_ghost_row[ghost_num] + 1)) && (current_pac_col == current_ghost_col[ghost_num])) ||
      ((pac_current_dir == MOVE_DOWN) && (current_pac_row == (current_ghost_row[ghost_num] - 1)) && (current_pac_col == current_ghost_col[ghost_num])))
    same_spot = true;

  return same_spot;

}

void reset_positions_counters_display()
{
  big_dot_eaten_counter = 0;
  /* Reset Counter */
  pac_counter = BIG_DOT_TIME_EFFECT_GHOST;
  
  /* Reset Ghost directions, positions */
  ghost_current_dir[0] = MOVE_RIGHT;
  ghost_current_dir[1] = MOVE_UP;
  ghost_current_dir[2] = MOVE_LEFT;
  current_ghost_row[0] = GHOST_HOUSE_ROW;
  current_ghost_col[0] = GHOST_HOUSE_COL - 3;
  current_ghost_row[1] = GHOST_HOUSE_ROW;
  current_ghost_col[1] = GHOST_HOUSE_COL;
  current_ghost_row[2] = GHOST_HOUSE_ROW;
  current_ghost_col[2] = GHOST_HOUSE_COL + 3;
  /* Reset Pac position */
  current_pac_row = PAC_START_ROW;
  current_pac_col = PAC_START_COL;
  /* Reset background display of maze */
  init_pac_display();
  delay(500);
}


/* Called each time there is a new level to re-init stuff */
void new_pac_level()
{
  unsigned char i = 0;
  /* Reset dots */
  for (i = 0; i < NUM_DOTS_ROWS; i++)
    current_dots[i] = 0x7F; /* Full row of dots */
  current_dots[2] = 0x63; // no dots in ghost home
  reset_positions_counters_display();
}

/* Defines allowable movements for Pac, Ghosts based on current location */
  // 0x1 - move left, 0x2 - move right
  // 0x4 - move up, 0x8 - move down
  // Assume bottom left corner of 2x2 sprite
const unsigned char PROGMEM pac_move[NUM_DISP_ROWS][NUM_DISP_COLS] =
  {{0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}, // 0
   {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}, // 1
   {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}, // 2
   {0x0,0xA,0x3,0x3,0xB,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0xB,0x3,0x3,0x9,0x0,0x0}, // 3
   {0x0,0xC,0x0,0x0,0xC,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xC,0x0,0x0,0xC,0x0,0x0}, // 4
   {0x0,0xC,0x0,0x0,0xC,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xC,0x0,0x0,0xC,0x0,0x0}, // 5
   {0x0,0xC,0x0,0x0,0xE,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0xD,0x0,0x0,0xC,0x0,0x0}, // 6
   {0x0,0xC,0x0,0x0,0xC,0x0,0x0,0x0,0x0,0x0,0x4,0x0,0x0,0x0,0x0,0x0,0xC,0x0,0x0,0xC,0x0,0x0}, // 7
   {0x0,0xC,0x0,0x0,0xC,0x0,0x0,0x0,0x0,0x0,0x4,0x0,0x0,0x0,0x0,0x0,0xC,0x0,0x0,0xC,0x0,0x0}, // 8
   {0x0,0xC,0x0,0x0,0xC,0x0,0x0,0x2,0x2,0x2,0x4,0x1,0x1,0x1,0x0,0x0,0xC,0x0,0x0,0xC,0x0,0x0}, // 9
   {0x0,0xC,0x0,0x0,0xC,0x0,0x0,0x2,0x2,0x2,0x4,0x1,0x1,0x1,0x0,0x0,0xC,0x0,0x0,0xC,0x0,0x0}, // 10
   {0x3,0xF,0x3,0x3,0xD,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xE,0x3,0x3,0xF,0x3,0x3}, // 11
   {0x0,0xC,0x0,0x0,0xC,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xC,0x0,0x0,0xC,0x0,0x0}, // 12
   {0x0,0xC,0x0,0x0,0xE,0x3,0x3,0x3,0x3,0x3,0xB,0x3,0x3,0x3,0x3,0x3,0xD,0x0,0x0,0xC,0x0,0x0}, // 13
   {0x0,0xC,0x0,0x0,0xC,0x0,0x0,0x0,0x0,0x0,0xC,0x0,0x0,0x0,0x0,0x0,0xC,0x0,0x0,0xC,0x0,0x0}, // 14
   {0x0,0xC,0x0,0x0,0xC,0x0,0x0,0x0,0x0,0x0,0xC,0x0,0x0,0x0,0x0,0x0,0xC,0x0,0x0,0xC,0x0,0x0}, // 15
   {0x0,0xC,0x0,0x0,0x6,0x3,0xB,0x3,0x3,0x3,0x7,0x3,0x3,0x3,0xB,0x3,0x5,0x0,0x0,0xC,0x0,0x0}, // 16
   {0x0,0xC,0x0,0x0,0x0,0x0,0xC,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xC,0x0,0x0,0x0,0x0,0xC,0x0,0x0}, // 17
   {0x0,0xC,0x0,0x0,0x0,0x0,0xC,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xC,0x0,0x0,0x0,0x0,0xC,0x0,0x0}, // 18
   {0x0,0x6,0x3,0x3,0x3,0x3,0x7,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x7,0x3,0x3,0x3,0x3,0x5,0x0,0x0}, // 19
   {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}, // 20
   {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}};// 21

void play_pac_man()
{
  bool pac_man_over = false;
  unsigned int move_dir = MOVE_NONE;
  unsigned int i, j;
  unsigned char ghost_mode;
  /* allow for "cornering", press button ahead of move */
  unsigned char loaded_move_dir = MOVE_NONE; 

  /* Reset the score */
  current_pac_level = 0;

  num_pac_lives = NUM_PAC_LIVES_START; 

  /* Display Splash Screen */
  displayPacStart();
  
  /* Initialized to new level */
  new_pac_level();

  delay(2500);

  while (pac_man_over == false)
  {
    move_dir = getMove();
    /* exit game on START */
    if (move_dir == MOVE_START)
      pac_man_over = true;

    /* Update Pac Counter */
    pac_counter++;

    /* Update Loaded Move Direction every cycle */
    if ((move_dir == MOVE_RIGHT) || (move_dir == MOVE_LEFT) || (move_dir == MOVE_UP) || (move_dir == MOVE_DOWN))
      loaded_move_dir = move_dir;

    /* Update Pac Man location */
    if (time_to_move(get_pac_speed()))
    {
      if (move_pac_man(loaded_move_dir, pgm_read_byte_near(&pac_move[current_pac_row][current_pac_col]))) /* if direction changed */
        loaded_move_dir = MOVE_NONE;
    }

    /* Get current Ghost Mode */
    ghost_mode = get_ghost_mode();
    /* Update Ghosts Positions */
    /* Update positions at rate based on current speed */
    if (time_to_move(get_ghost_speed()))
    {
      for (i = 0; i < NUM_GHOSTS; i++)
        move_ghost(i, pgm_read_byte_near(&pac_move[current_ghost_row[i]][current_ghost_col[i]]));
      previous_ghost_mode = ghost_mode;
    }

    /* Update dots eaten */
    if (((current_pac_col % 3) > 0) && ((current_pac_row % 3) < 2))
      if ((current_dots[(current_pac_row - 2) / 3] & (1 << ((current_pac_col) / 3))) > 0)
      {
        i = (current_pac_row - 2) / 3; // current pac row in dots 
        j = current_pac_col / 3; // current pac col in dots
        current_dots[i] = current_dots[i] ^ (1 << j);
        dot_eaten_counter = pac_counter; /* Dot takes effect next cycle */
        if (((i == 0) || (i == NUM_DOTS_ROWS - 1)) && ((j == 0) || (j == NUM_DOTS_COLS - 1)))
          big_dot_eaten_counter = pac_counter; /* Big Dot takes effect next cycle */
      }

    
    
    /* Update display items */
    clear_ghosts_pac();
    display_dots();
    display_pac_lives();
    display_pac_man();
    display_ghosts();
    displayLEDs(true, false);

    /* Look to see if all dots eaten and start new level if needed */
    j = 0; /* used to add up remaining dots on all rows */
    for (i = 0; i < NUM_DOTS_ROWS; i++)
      j += current_dots[i];
    if (j == 0) // new level
    {
      new_pac_level();
      current_pac_level++;
    }

    /* If ghost catches you, quit or eat*/
    for (i = 0; i < NUM_GHOSTS; i++)
    {
      if (occupy_same_spot(i))
      {
        if (ghost_mode == GHOST_MODE_FRIGHT)
        {
          /* Just place in middle of ghost house */
          current_ghost_row[i] = GHOST_HOUSE_ROW;
          current_ghost_col[i] = GHOST_HOUSE_COL;
        }
        else
        {
          num_pac_lives--;
          if (num_pac_lives > 0)
            reset_positions_counters_display();
          else
            pac_man_over = true;
        }
        delay(500);
      }
    }

    /* If completd all levels, quit */
    if (current_pac_level > NUM_PAC_LEVELS)
      pac_man_over = true;

    
    delay(10);
  }
  /* Display Splash Screen */
  displayPacStart();
  displayScore(current_pac_level, true);
  delay(2500); /* Delay so START button press is only picked up once */
}


/* Game Menu */

unsigned char game_selection = 0; /* game selected: 0 - PAC, 1 - TET, 2 - MAR */

/* Game Menu Display */
const unsigned char PROGMEM gameMenuDisp[NUM_DISP_ROWS_MENU - 3][NUM_DISP_COLS_MENU] =
  {{0,0,0,5,5,5,4,4,4,0,5,5},
   {0,0,0,5,0,5,4,0,4,5,0,0},
   {0,0,0,5,5,5,4,4,4,5,0,0},
   {0,0,0,5,0,0,4,0,4,5,0,0},
   {0,0,0,5,0,0,4,0,4,0,5,5},
   {0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,4,4,4,5,5,5,4,4,4},
   {0,0,0,0,4,0,5,0,0,0,4,0},
   {0,0,0,0,4,0,5,5,5,0,4,0},
   {0,0,0,0,4,0,5,0,0,0,4,0},
   {0,0,0,0,4,0,5,5,5,0,4,0},
   {0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,5,0,5,4,4,4,5,5,5},
   {0,0,0,5,5,5,4,0,4,5,0,5},
   {0,0,0,5,0,5,4,4,4,5,5,0},
   {0,0,0,5,0,5,4,0,4,5,5,5},
   {0,0,0,5,0,5,4,0,4,5,0,5}};

void displayGameMenu()
{
  const unsigned int unused_rows_top = (NUM_DISP_ROWS - NUM_DISP_ROWS_MENU) / 2;
  const unsigned int unused_cols_left = (NUM_DISP_COLS - NUM_DISP_COLS_MENU) / 2;
  unsigned int i, j;
  
  for(i = 0; i < NUM_DISP_ROWS; i++)
    for(j = 0; j < NUM_DISP_COLS; j++)
      bigDispBoard[i][j] = DISP_COLOR_BLACK;
    
  for(i = 0; i < NUM_DISP_ROWS_MENU - 3; i++)
    for(j = 0; j < NUM_DISP_COLS_MENU; j++)
      bigDispBoard[i + unused_rows_top][j + unused_cols_left] = pgm_read_byte_near(&gameMenuDisp[i][j]);

  /* Display Selection Cursor */
  bigDispBoard[unused_rows_top + 1 + 6 * game_selection][unused_cols_left] = DISP_COLOR_YELLOW;
  bigDispBoard[unused_rows_top + 1 + 6 * game_selection][unused_cols_left + 1] = DISP_COLOR_YELLOW;
  bigDispBoard[unused_rows_top + 2 + 6 * game_selection][unused_cols_left] = DISP_COLOR_YELLOW;
  bigDispBoard[unused_rows_top + 2 + 6 * game_selection][unused_cols_left + 1] = DISP_COLOR_YELLOW;

  displayLEDs(true, false);

}





unsigned char mario_play_now = 0;

void start_menu()
{
  bool game_selected = false; 
  unsigned int move_dir = MOVE_NONE;

  displayGameMenu();

  while (game_selected == false)
  {
    move_dir = getMove();

    if (move_dir == MOVE_UP)
      game_selection = (game_selection + 2) % 3;
    else if (move_dir == MOVE_DOWN)
      game_selection = (game_selection + 1) % 3;
    else if ((move_dir >> 4) == MOVE_ROTATE_RIGHT)
      game_selected = true;
    else if (( move_dir >> 4) == MOVE_ROTATE_LEFT)
      game_selected = true;
    else if (move_dir == MOVE_SELECT) // exit menu
    {
      game_selection = 5;
      game_selected = true;
    }
      

    /* Re-display background */
    displayGameMenu();

    delay(80);
  }

  if (game_selection == 0)
    play_pac_man(); 
  else if (game_selection == 1)
    play_tetris();
  else if (game_selection == 2)
  {
    display_mario_run();
    mario_play_now = disp_mario_luigi + 1;
  }
  delay(300);
  
}





void run_paint()
{
  bool paint_ended = false; 
  unsigned int move_dir = MOVE_NONE;
  unsigned char current_row = 11; // 1 - 20
  unsigned char current_col = 11; // 1 - 20
  unsigned char current_color = 0; // 0 - 7
  unsigned char previous_space_color = 0; // 0 - 7

  for(int i = 0; i < NUM_DISP_ROWS; i++)
    for(int j = 0; j < NUM_DISP_COLS; j++)
      bigDispBoard[i][j] = DISP_COLOR_BLACK;

  while (paint_ended == false)
  {
    move_dir = getMove();

    if (move_dir == MOVE_UP)
    {
      bigDispBoard[current_row][current_col] = previous_space_color;
      current_row--;
      if (current_row == 0)
        current_row = 20;
      previous_space_color = bigDispBoard[current_row][current_col];
    }
    else if (move_dir == MOVE_DOWN)
    {
      bigDispBoard[current_row][current_col] = previous_space_color;
      current_row++;
      if (current_row == 21)
        current_row = 1;
      previous_space_color = bigDispBoard[current_row][current_col];
    }
    else if (move_dir == MOVE_RIGHT)
    {
      bigDispBoard[current_row][current_col] = previous_space_color;
      current_col++;
      if (current_col == 21)
        current_col = 1;
      previous_space_color = bigDispBoard[current_row][current_col];
    }
    else if (move_dir == MOVE_LEFT)
    {
      bigDispBoard[current_row][current_col] = previous_space_color;
      current_col--;
      if (current_col == 0)
        current_col = 20;
      previous_space_color = bigDispBoard[current_row][current_col];
    }
    else if ((move_dir >> 4) == MOVE_ROTATE_RIGHT)
    {
      bigDispBoard[current_row][current_col] = pgm_read_byte_near(&paintColors[current_color]);
      previous_space_color = bigDispBoard[current_row][current_col];
    }
    else if (( move_dir >> 4) == MOVE_ROTATE_LEFT)
      current_color = (current_color + 1) % NUM_PAINT_COLORS;
    else if (move_dir == MOVE_START)
      paint_ended = true;

    // draw cursor
    light_twinkle = (light_twinkle + 1) % 4;
    if (light_twinkle < 2)
      bigDispBoard[current_row][current_col] = pgm_read_byte_near(&paintColors[current_color]);
    else
    {
      bigDispBoard[current_row][current_col] = 0;
      if (current_color == NUM_PAINT_COLORS - 1) // Black (Erase)
        bigDispBoard[current_row][current_col] = 5;
    }
      

    if (current_color == NUM_PAINT_COLORS - 1) // Black (Erase)
    {
      for(int j = 1; j < NUM_DISP_COLS; j++)
      {
        bigDispBoard[0][j] = 5;
      }
    }
    else
    {
      for(int j = 1; j < NUM_DISP_COLS; j++)
      {
        bigDispBoard[0][j] = 0;
      }
    }

    // draw
    displayLEDs(true, false);

    delay(80);
  }
  
}



// TREE (Start at bottom right and wind around the tree upward)
// Row  13: BeforeLEDs, leftmost, ..., rightmost, 
// Row ...: BeforeLEDs, leftmost, ..., rightmost, 
// Row   1: BeforeLEDs(50-59), leftmost(49), ..., rightmost(40), 
// Row   0: BeforeLEDs(20-29), leftmost(19), ..., rightmost(10), 
/* Convert from bigDispBoard to LEDs on Tree */
/* Start at bottom, right and wind around tree */
//void displayLEDTreeCal()
//{
//  unsigned int i, j;
//  unsigned int currentLED = NUM_LEDS;
//  unsigned int unused_rows_top = (NUM_DISP_ROWS - num_tree_rows) / 2 + num_tree_rows;
//  unsigned int unused_cols_left = (NUM_DISP_COLS - num_tree_cols) / 2 + num_tree_cols;
//  
//  /* bottom row to top, right to left */
//  for(i = 0; i < num_tree_rows; i++)
//  {
//    /* ledsBeforeRows starts at bottom row of tree, right to left */
//    for(j = 0; j < ledsBeforeRows[i]; j++)
//    {      
//      leds[currentLED] = CRGB::Black;
//      currentLED++;
//    }
//
//    for(j = num_tree_cols - 1; j >= 0; j--)
//    { 
//      if ((i % 2) == 0)     
//        leds[currentLED] = DISP_COLOR_GREEN;
//      else
//        leds[currentLED] = DISP_COLOR_RED;
//      currentLED++;
//    }
//  }
//
//  for(i = currentLED; i < NUM_TREE_LEDS; i++)
//    leds[currentLED] = CRGB::Black;
//
//  FastLED.show();
//}


/* Calibration Num Rows Display */
//const unsigned char PROGMEM calNumRowsDisp[11][NUM_DISP_COLS_TETRIS] =
//  {{5,5,5,4,0,4,5,0,5,0},
//   {5,0,5,4,0,4,5,5,5,0},
//   {5,0,5,4,0,4,5,0,5,0},
//   {5,0,5,4,0,4,5,0,5,0},
//   {5,0,5,4,4,4,5,0,5,0},
//   {0,0,0,0,0,0,0,0,0,0},
//   {5,5,5,4,4,4,5,0,5,0},
//   {5,0,5,4,0,4,5,0,5,0},
//   {5,5,0,4,0,4,5,0,5,0},
//   {5,5,5,4,0,4,5,5,5,0},
//   {5,0,5,4,4,4,5,0,5,0}};
//
///* Calibration Num Cols Display */
//const unsigned char PROGMEM calNumColsDisp[11][NUM_DISP_COLS_TETRIS] =
//  {{5,5,5,4,0,4,5,0,5,0},
//   {5,0,5,4,0,4,5,5,5,0},
//   {5,0,5,4,0,4,5,0,5,0},
//   {5,0,5,4,0,4,5,0,5,0},
//   {5,0,5,4,4,4,5,0,5,0},
//   {0,0,0,0,0,0,0,0,0,0},
//   {5,5,5,4,4,4,5,0,0,0},
//   {5,0,0,4,0,4,5,0,0,0},
//   {5,0,0,4,0,4,5,0,0,0},
//   {5,0,0,4,0,4,5,0,0,0},
//   {5,5,5,4,4,4,5,5,5,0}};
//
//void dispCalNumRowsCols(bool isRows) /* isRows=true => Rows, else Cols */
//{
//  /* Use Tetris Playfield to display Pac Man Start screen */
//  unsigned int unused_rows_top = (NUM_DISP_ROWS - NUM_DISP_ROWS_TETRIS) / 2;
//  unsigned int unused_cols_left = (NUM_DISP_COLS - NUM_DISP_COLS_TETRIS) / 2;
//  unsigned char i,j;
//    
//  for(i = 0; i < 11; i++)
//    for(j = 0; j < NUM_DISP_COLS_TETRIS; j++)
//    { 
//      if (isRows == true)
//        bigDispBoard[i + unused_rows_top][j + unused_cols_left] = pgm_read_byte_near(&calNumRowsDisp[i][j]);
//      else
//        bigDispBoard[i + unused_rows_top][j + unused_cols_left] = pgm_read_byte_near(&calNumColsDisp[i][j]);
//    }
//
//  displayBigBoardTwoTwo(true);
//}

/* Calibration Num Cols Display */
//const unsigned char PROGMEM calRowOffset[5][NUM_DISP_COLS_TETRIS] =
//  {{5,5,5,4,4,4,5,0,5,0},
//   {5,0,5,4,0,4,5,0,5,0},
//   {5,5,0,4,0,4,5,0,5,0},
//   {5,5,5,4,0,4,5,5,5,0},
//   {5,0,5,4,4,4,5,0,5,0}};
//
//void dispLedsCal(unsigned char row_num)
//{
//  unsigned char i,j;
//  unsigned int unused_rows_top = (NUM_DISP_ROWS - NUM_DISP_ROWS_TETRIS) / 2;
//  unsigned int unused_cols_left = (NUM_DISP_COLS - NUM_DISP_COLS_TETRIS) / 2;
//  unsigned int tens_digit = (row_num % 100) / 10;
//  unsigned int ones_digit = row_num % 10;
//    
//  for(i = 0; i < 5; i++)
//    for(j = 0; j < NUM_DISP_COLS_TETRIS; j++)
//      bigDispBoard[i + unused_rows_top][j + unused_cols_left] = pgm_read_byte_near(&calRowOffset[i][j]);
//
//  /* Display row number */
//  /* Display Score (number of lines) */
//  for(j = 0; j < 5; j++) // for each column
//  {
//    if (((pgm_read_byte(&left_column[tens_digit]) >> j) & 0x1) > 0)
//      bigDispBoard[7+j][9] = 4; // Red
//    if (((pgm_read_byte(&middle_column[tens_digit]) >> j) & 0x1) > 0)
//      bigDispBoard[7+j][10] = 4; // Red
//    if (((pgm_read_byte(&right_column[tens_digit]) >> j) & 0x1) > 0)
//      bigDispBoard[7+j][11] = 4; // Red
//    if (((pgm_read_byte(&left_column[ones_digit]) >> j) & 0x1) > 0)
//      bigDispBoard[7+j][13] = 5; // Green
//    if (((pgm_read_byte(&middle_column[ones_digit]) >> j) & 0x1) > 0)
//      bigDispBoard[7+j][14] = 5; // Green
//    if (((pgm_read_byte(&right_column[ones_digit]) >> j) & 0x1) > 0)
//      bigDispBoard[7+j][15] = 5; // Green 
//  }
//
//  displayBigBoardTwoTwo(true);
//}




//void calibrate_tree()
//{
//  bool cal_complete = false;
//  unsigned int move_dir = MOVE_NONE;
//  unsigned int i;
//  unsigned char j, k;
//
//  for(i = NUM_LEDS; i < (NUM_LEDS  + NUM_TREE_LEDS); i++)
//     leds[i] = CRGB::Black;
//  FastLED.show();
//     
//  
//  /* First, determine number of display rows */
//  while (cal_complete == false)
//  {
//    move_dir = getMove();
//
//    /* range from 10 - NUM_DISP_ROWS_TREE */
//    if (move_dir == MOVE_UP)
//      num_tree_rows = ((num_tree_rows + 2) % (NUM_DISP_ROWS_TREE + 2)) % (NUM_DISP_ROWS_TREE - 9) + 10;
//    else if (move_dir == MOVE_DOWN)
//      num_tree_rows = ((num_tree_rows + NUM_DISP_ROWS - 1) % (NUM_DISP_ROWS_TREE + 1)) % (NUM_DISP_ROWS_TREE - 9) + 10;
//    else if (move_dir == MOVE_START)
//      cal_complete = true;
//
//    for(j = 0; j < NUM_DISP_ROWS; j++)
//        for(k = 0; k < NUM_DISP_COLS; k++)
//            bigDispBoard[j][k] = DISP_COLOR_WHITE;
//        
//    /* Display Num Rows (stored in EEPROM) */
//    displayScore(num_tree_rows, false);
//    /* Re-display background */
//    dispCalNumRowsCols(true);
//    /* Display updated tree board */
//    displayLEDTreeCal();
//    
//
//    delay(80);
//  }
//  /* write num_rows to EEPROM */
//  EEPROM.write(NUM_ROWS_EEPROM_ADDRESS, num_tree_rows);
//  delay(250);
//  cal_complete = false;
//
//  /* Next, determine number of display columns */
//  while (cal_complete == false)
//  {
//    move_dir = getMove();
//
//   /* range from 10 - NUM_DISP_ROWS_TREE */
//    if (move_dir == MOVE_UP)
//      num_tree_cols = ((num_tree_cols + 2) % (NUM_DISP_COLS_TREE + 2)) % (NUM_DISP_COLS_TREE - 9) + 10;
//    else if (move_dir == MOVE_DOWN)
//      num_tree_cols = ((num_tree_cols + NUM_DISP_COLS - 1) % (NUM_DISP_COLS_TREE + 1)) % (NUM_DISP_COLS_TREE - 9) + 10;
//    else if (move_dir == MOVE_START)
//      cal_complete = true;
//
//    for(j = 0; j < NUM_DISP_ROWS; j++)
//        for(k = 0; k < NUM_DISP_COLS; k++)
//            bigDispBoard[j][k] = DISP_COLOR_WHITE;
//
//    /* Display Num Cols (stored in EEPROM) */
//    displayScore(num_tree_cols, false);
//    /* Re-display background */
//    dispCalNumRowsCols(false);
//    /* Display updated tree board */
//    displayLEDTreeCal();
//    
//
//    delay(80);
//  }
//  /* write num_cols to EEPROM */
//  EEPROM.write(NUM_COLS_EEPROM_ADDRESS, num_tree_cols);
//  delay(250);
//  cal_complete = false;
//
//  /* Finally get offsets for each row (number of leds before each row, starting
//     at bottom right and going up-spiral) */
//  for(i = 0; i < num_tree_rows; i++)
//  {
//    cal_complete = false;
//    while (cal_complete == false)
//    {
//      move_dir = getMove();
//
//      if (move_dir == MOVE_UP)
//        ledsBeforeRows[i] = (ledsBeforeRows[i] + 1) % MAX_LED_OFFSET;
//      else if (move_dir == MOVE_DOWN)
//        ledsBeforeRows[i] = (ledsBeforeRows[i] + MAX_LED_OFFSET - 1) % MAX_LED_OFFSET;
//      else if (((i % 2) == 1) && (move_dir == MOVE_SELECT))
//        cal_complete = true;
//
//      for(j = 0; j < NUM_DISP_ROWS; j++)
//        for(k = 0; k < NUM_DISP_COLS; k++)
//           bigDispBoard[j][k] = DISP_COLOR_WHITE;
//
//      /* Display Num Cols (stored in EEPROM) */
//      displayScore(ledsBeforeRows[i], false);
//      /* Re-display background */
//      dispLedsCal(i);
//      /* Display updated tree board */
//      displayLEDTreeCal();
//
//      delay(80);
//    } /* end while loop for this row */
//    /* Write EEPROM led offset value for this row */
//    EEPROM.write(ROW_OFFSET_EEPROM_ADDRESS + i, ledsBeforeRows[i]);
//    delay(250);
//  }
//  
//}

/* Picture One Display */
const unsigned char PROGMEM picOneDisp[NUM_DISP_ROWS_MENU][19] =
  {{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   { 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0},
   { 0, 0,19,19, 4, 4, 4, 4, 0, 0, 0, 4, 4, 4, 4,19,19, 0, 0},
   { 0,19,19,19, 4, 4, 4, 4, 4, 0, 4, 4, 4, 4, 4,19,19,19, 0},
   {19,19,19,19,19, 4, 4, 4, 4, 4, 4, 4, 4, 4,19,19,19,19,19},
   {19,19,19,19,19, 4, 4, 4, 4, 4, 4, 4, 4, 4,19,19,19,19,19},
   {19,19,19,19, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,19,19,19,19},
   { 0,19,19,19, 4, 4, 4, 4, 4,19, 4, 4, 4, 4, 4,19,19,19, 0},
   { 0, 0,19,19,19, 4, 4,19,19,19,19,19, 4, 4,19,19,19, 0, 0},
   { 0, 0, 0, 0,19,19,19,19,19,19,19,19,19,19,19, 0, 0, 0, 0},
   { 0, 0, 0,19,19,19,19,19,19,19,19,19,19,19,19,19, 0, 0, 0},
   { 0, 0, 0,19,19,19,19,19,19,19,19,19,19,19,19,19, 0, 0, 0},
   { 0, 0, 0,19,19,19,19,19,19,19,19,19,19,19,19,19, 0, 0, 0},
   { 0, 0, 0, 0,19,19,19,19,19,19,19,19,19,19,19, 0, 0, 0, 0},
   { 0, 0, 0, 0,19,19,19,19,19,19,19,19,19,19,19, 0, 0, 0, 0},
   { 0, 0, 0, 0, 0,19,19,19,19,19,19,19,19,19, 0, 0, 0, 0, 0},
   { 0, 0, 0, 0, 0, 0, 0,19,19,19,19,19, 0, 0, 0, 0, 0, 0, 0},
   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

//const unsigned char PROGMEM picTwoDisp[NUM_DISP_ROWS_MENU][17] =
//  {{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//   { 0, 0, 0, 0, 0, 0,26,26, 0,26,26, 0, 0, 0, 0, 0, 0},
//   { 0, 0, 0, 0,26,26,26,26, 0,26,26,26,26, 0, 0, 0, 0},
//   { 0, 0, 0,26,26,26,26,26, 0,26,26,26,26,26, 0, 0, 0},
//   { 0, 0,26, 0,26,26,26,26, 0,26,26,26,26, 0,26, 0, 0},
//   { 0,26,26,26, 0,26,26,26, 0,26,26,26, 0,26,26,26, 0},
//   { 0,26,26,26,26, 0,26,26, 0,26,26, 0,26,26,26,26, 0},
//   {26,26,26,26,26, 0,26,26, 0,26,26, 0,26,26,26,26,26},
//   {26,26,26,26,26, 0,26,26, 0,26,26, 0,26,26,26,26,26},
//   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//   {26,26,26,26,26, 0,26,26, 0,26,26, 0,26,26,26,26,26},
//   {26,26,26,26,26, 0,26,26, 0,26,26, 0,26,26,26,26,26},
//   { 0,26,26,26,26, 0,26,26, 0,26,26, 0,26,26,26,26, 0},
//   { 0,26,26,26, 0,26,26,26, 0,26,26,26, 0,26,26,26, 0},
//   { 0, 0,26, 0,26,26,26,26, 0,26,26,26,26, 0,26, 0, 0},
//   { 0, 0, 0,26,26,26,26,26, 0,26,26,26,26,26, 0, 0, 0},
//   { 0, 0, 0, 0,26,26,26,26, 0,26,26,26,26, 0, 0, 0, 0},
//   { 0, 0, 0, 0, 0, 0,26,26, 0,26,26, 0, 0, 0, 0, 0, 0},
//   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

void display_picture_one()
{
  const unsigned int unused_rows_top = (NUM_DISP_ROWS - NUM_DISP_ROWS_MENU) / 2;
  const unsigned int unused_cols_left = (NUM_DISP_COLS - 19) / 2;
  unsigned char i,j;
  for(i = 0; i < NUM_DISP_ROWS_MENU; i++)
    for(j = 0; j < 19; j++)
      bigDispBoard[i + unused_rows_top][j + unused_cols_left] = pgm_read_byte_near(&picOneDisp[i][j]);
  displayLEDs(true, false);
  delay(3000);
  display_clear();
}

//void display_picture_two()
//{
//  const unsigned int unused_rows_top = (NUM_DISP_ROWS - NUM_DISP_ROWS_MENU) / 2;
//  const unsigned int unused_cols_left = (NUM_DISP_COLS - 17) / 2;
//  unsigned char i,j;
//  for(i = 0; i < NUM_DISP_ROWS_MENU; i++)
//    for(j = 0; j < 17; j++)
//      bigDispBoard[i + unused_rows_top][j + unused_cols_left] = pgm_read_byte_near(&picTwoDisp[i][j]);
//  displayLEDs(true, false);
//  delay(3000);
//}



/* Main Function */
unsigned char color1_in = 3, color2_in = 4;

unsigned char display_mode = DISP_LIGHT;
//unsigned char calibration_mode = 0;
void loop() {
  unsigned int moveDir = MOVE_NONE;
  unsigned char i, j;
  unsigned char num_leds = 0;
  const unsigned int unused_rows_top = (NUM_DISP_ROWS - NUM_DISP_ROWS_MENU) / 2;
  const unsigned int unused_cols_left = (NUM_DISP_COLS - NUM_DISP_COLS_MENU) / 2;
  //digitalWrite(MUSIC_PIN_TETRIS, LOW);
  //digitalWrite(MUSIC_PIN_PAC, LOW);
  //digitalWrite(MUSIC_PIN_MARIO, LOW);
  
  /* Monitor Inputs */
  moveDir = getMove();

  /* Move up and down between display options */
  if (moveDir == MOVE_UP)
  {
    display_mode = (display_mode + NUM_DISP_MODES - 1) % NUM_DISP_MODES;
    display_clear();
    color1_in = 3;
    color2_in = 4;
  }
  else if (moveDir == MOVE_DOWN)
  {
    display_mode = (display_mode + 1) % NUM_DISP_MODES;
    display_clear();
    color1_in = 3;
    color2_in = 4;
  }
  else if (moveDir == MOVE_LEFT)
  {
    color1_in = (color1_in + 1) % NUM_PAINT_COLORS;
  }
  else if (moveDir == MOVE_RIGHT)
  {
    color2_in = (color2_in + 1) % NUM_PAINT_COLORS;
  }
  else if (( moveDir >> 4) == MOVE_ROTATE_LEFT)
  {
    display_clear();
    run_paint();
    moveDir = MOVE_NONE;
  }
  else if (( moveDir >> 4) == MOVE_ROTATE_RIGHT)
  {
    display_clear();
    play_gimkit();
    moveDir = MOVE_NONE;
  }
  


  /* Update display for current selection */
  /* If tetris, init and kick out of loop */
  if (moveDir == MOVE_START) //|| (digitalRead(RESET_SWITCH_IN) == HIGH))
    start_menu();

  if (mario_play_now > 0)
  {
    init_mario();
    unsigned int mario_score = 0;

    // play the game
    mario_score = play_mario(mario_play_now);
    display_mario_end_animation();
    
    /* display score */
    for(i = 0; i < NUM_DISP_ROWS; i++)
    for(j = 0; j < NUM_DISP_COLS; j++)
      bigDispBoard[i][j] = DISP_COLOR_BLACK;
    
    for(i = 12; i < 17; i++)
      for(j = 0; j < NUM_DISP_COLS_MENU; j++)
        bigDispBoard[i + unused_rows_top - 12][j + unused_cols_left] = pgm_read_byte_near(&gameMenuDisp[i][j]);

    if (mario_score > 199)
      mario_score = 199;
    displayScore(mario_score, true);
    delay(3000);
    mario_play_now = 0;
  }
    
  else if (moveDir == MOVE_SELECT) /* Calibrate if SELECT, A, B pressed in order */
  {
    display_clear();
//    calibration_mode = 1;
    display_picture_one();
  }
//  else if ((moveDir >> 4 == MOVE_ROTATE_RIGHT) && (calibration_mode == 1))
//    calibration_mode++;
//  else if ((moveDir >> 4 == MOVE_ROTATE_LEFT) && (calibration_mode == 2))
//  {
//    calibrate_tree();
//    calibration_mode = 0;
//  }
  else if (display_mode == DISP_GR)
    display_green_red(color1_in, color2_in);
  else if (display_mode == DISP_LIGHT)
    display_lights();
  else if (display_mode == DISP_ONE_COLOR)
    display_one_color();
  else if (display_mode == DISP_RAINBOW)
    display_rainbow();
  else if (display_mode == DISP_JUGGLE)
    display_juggle();
  else if (display_mode == DISP_CASTLE)
    display_castle();  
  else if (display_mode == DISP_CAL_BOARD)
    display_cal_board(); 
  else if (display_mode == DISP_VORTEX)
    display_vortex(pgm_read_byte_near(&paintColors[color1_in]), pgm_read_byte_near(&paintColors[color2_in]));
  else if (display_mode == DISP_ROWS_MOVE)
    display_rows_move(pgm_read_byte_near(&paintColors[color1_in]), pgm_read_byte_near(&paintColors[color2_in]));
  else if (display_mode == DISP_VORTEX_3)
    //display_vortex_2(0x008080, 0x00ff80); 
    display_every_other();
  else if (display_mode == DISP_SPIRAL_1)
    display_spiral(pgm_read_byte_near(&paintColors[color1_in]), pgm_read_byte_near(&paintColors[color2_in]), pgm_read_byte_near(&paintColors[color1_in]), pgm_read_byte_near(&paintColors[color2_in]));
  else if (display_mode == DISP_SPIRAL_2)
    display_spiral(19, 8, 1, 4);
//  else if (digitalRead(PICTURE_INPUT_1) == HIGH)
//    display_picture_one();
//  else if (digitalRead(PICTURE_INPUT_2) == HIGH)
//    display_picture_two();
    
  delay(50);

}
