#include <FastLED.h>
#include <EEPROM.h>
#include <avr/pgmspace.h>

/* TODO: */
/* Mario */


/*  Display everything on a 22x22 board, top left is 0  
 *  Convert board to 22x22 snake display, 20x10 snake display, 
 *  or xmas tree as dictated by #define display mode below
 */

#define NUM_TREE_LEDS    550

//#ifdef TEN_X_TWENTY
//#define NUM_LEDS    200
//#endif

#define NUM_LEDS    484

/* CONTROL MODES */
#define DISP_LIGHT     0
#define DISP_GR        1
#define DISP_RAINBOW   2
#define DISP_JUGGLE    3
#define DISP_CASTLE    4
#define DISP_ONE_COLOR 5
#define DISP_SNAKE     6
#define NUM_DISP_MODES 7



#define BRIGHTNESS  5
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB



/* Number of rows, cols on the Big Display */
#define NUM_DISP_ROWS 22 
#define NUM_DISP_COLS 22


/* Max number of display rows, cols for xmas tree */
#define NUM_DISP_ROWS_TREE 20 /*middle twenty are visible */
// NUM_DISP_ROWS 20 for full tetris
#define NUM_DISP_COLS_TREE 20 /* middle ten are visible for tetris */

#define NUM_DISP_ROWS_TETRIS 20 /*middle twenty are visible */
// NUM_DISP_ROWS 20 for full tetris
#define NUM_DISP_COLS_TETRIS 10 /* middle ten are visible */

#define NUM_DISP_ROWS_MENU 20 /*middle twenty are visible */
// NUM_DISP_ROWS 20 for full tetris
#define NUM_DISP_COLS_MENU 12 /* middle ten are visible */

#define NUM_ROWS_MARIO_RUN 17 
#define NUM_COLS_MARIO_RUN 17 


#define DISP_COLOR_BLACK       0
#define DISP_COLOR_BLUE        1
#define DISP_COLOR_ORANGE      2
#define DISP_COLOR_YELLOW      3
#define DISP_COLOR_RED         4
#define DISP_COLOR_GREEN       5
#define DISP_COLOR_CYAN        6
#define DISP_COLOR_PEACH       7
#define DISP_COLOR_PURPLE      8
#define DISP_COLOR_LIGHT_BLUE 10
#define DISP_COLOR_WHITE      11
#define DISP_COLOR_GRAY       12
#define DISP_COLOR_BROWN      13
#define DISP_COLOR_PAC_BLUE   14
#define DISP_COLOR_HALF_WHITE 15
#define DISP_COLOR_HALF_BLUE  16
#define DISP_COLOR_HALF_RED   17
#define DISP_COLOR_PAC_DOT    18
#define DISP_COLOR_PINK       19
#define DISP_COLOR_AQUA       20
#define NUM_DISP_COLORS       21
const CRGB numToColor[NUM_DISP_COLORS] = 
{CRGB::Black, CRGB::Blue, CRGB::Orange, CRGB::Yellow, CRGB::Red, CRGB::Green, CRGB::Cyan, CRGB::PeachPuff,
 CRGB::Purple, 0xD7FF00, 0x002332, CRGB::White, CRGB::Gray, 0x2F1010, 0x0000A0, 0x808080, 0x000080, 0x800000, 0x404040, 0xff69b4, 0x00cccc};

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





/* Num display rows and cols for xmas tree */
/* initial values, overwritten by values from EEPROM */
unsigned char num_tree_rows = NUM_DISP_ROWS_TREE;
unsigned char num_tree_cols = NUM_DISP_COLS_TREE;

/* Number of "unused" LEDs before rows, starting at top*/
unsigned char ledsBeforeRows[NUM_DISP_ROWS_TREE] =
{0,0,0,0,0,
 15,14,17,19,
 19,24,23,26,
 28,30,35,40,
 43,13};


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

//{{0,0,0,0,0,0,0,0,0,0},{6,6,6,0,0,0,0,0,0,0},{0,6,0,0,0,0,0,0,0,0},{0,6,4,4,0,0,0,0,0,0},
  // {0,6,4,0,0,0,0,0,0,0},{0,6,4,4,0,0,0,0,0,0},{0,0,4,8,8,8,0,0,0,0},{0,0,4,4,8,0,0,0,0,0},
  // {0,0,0,0,8,3,3,3,0,0},{0,0,0,0,8,3,0,3,0,0},{0,0,0,0,8,3,3,0,0,0},{0,0,0,0,0,3,3,0,0,0},
  // {0,0,0,0,0,3,0,3,0,0},{0,0,0,0,0,5,5,5,0,0},{0,0,0,0,0,0,5,1,1,1},{0,0,0,0,0,0,5,1,0,0},
  // {0,0,0,0,0,0,5,1,1,1},{0,0,0,0,0,5,5,5,0,1},{0,0,0,0,0,0,0,1,1,1},{0,0,0,0,0,0,0,0,0,0}};




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
#define CHANCE_OF_TWINKLE 12


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
CRGB leds[NUM_LEDS];
CRGB leds_tree[NUM_LEDS];

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
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<LED_TYPE, LED_TREE_PIN, COLOR_ORDER>(leds, NUM_TREE_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
  
  Serial.begin(9600);

  /* Change to seed when user presses START */
  randomSeed(analogRead(0));

  // Set appropriate pins to inputs
  pinMode(NES_DATA, INPUT);
  
  // Set appropriate pins to outputs
  pinMode(NES_CLOCK, OUTPUT);
  pinMode(NES_LATCH, OUTPUT);
  pinMode(RESET_SWITCH_IN, INPUT);
  pinMode(RESET_SWITCH_OUT, OUTPUT);
  pinMode(POWER_SWITCH_IN, INPUT);
  pinMode(POWER_SWITCH_OUT, OUTPUT);
  pinMode(MUSIC_PIN_TETRIS, OUTPUT);
  pinMode(MUSIC_PIN_PAC, OUTPUT);
  pinMode(MUSIC_PIN_MARIO, OUTPUT);

  
  // Set initial states
  digitalWrite(NES_CLOCK, LOW);
  digitalWrite(NES_LATCH, LOW);
  digitalWrite(RESET_SWITCH_OUT, HIGH);
  digitalWrite(POWER_SWITCH_OUT, HIGH);
  digitalWrite(MUSIC_PIN_TETRIS, LOW);
  digitalWrite(MUSIC_PIN_PAC, LOW);
  digitalWrite(MUSIC_PIN_MARIO, LOW);

  /* Read values from EEPROM */
  unsigned char i = EEPROM.read(NUM_ROWS_EEPROM_ADDRESS); /* num rows to be stored at address 0 */
  if ((i > 0) && (i <= NUM_DISP_ROWS_TREE))
    num_tree_rows = i;
  i = EEPROM.read(NUM_COLS_EEPROM_ADDRESS); /* num cols to be stored at address 0 */
  if ((i > 0) && (i <= NUM_DISP_COLS_TREE))
    num_tree_cols = i; 

  /* Update ledsBeforeRows values if something stored there > 0 */
  /* Right now these are stored from top to bottom */
  for (i = 0; i < NUM_DISP_ROWS_TREE; i++);
  {
    unsigned char num_leds = EEPROM.read(ROW_OFFSET_EEPROM_ADDRESS + i);
    if ((num_leds > 0) && (num_leds < 50))
      ledsBeforeRows[i] = num_leds;
  }
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
void displayLEDTree(bool showLed)
{
  int i, j;
  unsigned int currentLED = 0;
  const unsigned char num_rows = NUM_DISP_ROWS_TETRIS;//EEPROM.read(NUM_ROWS_EEPROM_ADDRESS); /* num rows to display */
  //const unsigned char num_cols = EEPROM.read(NUM_COLS_EEPROM_ADDRESS); /* num cols to display */
  
  unsigned int unused_cols_left = (NUM_DISP_ROWS - NUM_DISP_ROWS_TETRIS) / 2;
  unsigned int unused_rows_top = (NUM_DISP_COLS - NUM_DISP_COLS_TETRIS) / 2 + (NUM_DISP_ROWS_TETRIS - num_rows);
  
  //Serial.println("TREE");
  
  /* bottom row to top, right to left */
  //for(i = NUM_DISP_ROWS_TETRIS - 1; i >= 0; i--)
  for(i = num_rows - 1; i >= 0; i--)
  {
    //Serial.println("New Row");
    //Serial.println(i);
    
    for(j = ledsBeforeRows[i + (NUM_DISP_ROWS_TETRIS - num_rows)] - 1; j >= 0; j--)
    {      
      if (showLed == true)
        leds_tree[currentLED] = CRGB::Black;
      currentLED++;
    }
    //Serial.println(currentLED);
    for(j = NUM_DISP_COLS_TETRIS - 1; j >= 0; j--)
    {      
      leds_tree[currentLED] = numToColor[bigDispBoard[i + unused_rows_top][j + unused_cols_left]];
      currentLED++;
    }
  }
  //Serial.println(currentLED);
  for(i = currentLED; i < NUM_TREE_LEDS; i++)
    if (showLed == true)
        leds_tree[currentLED] = CRGB::Black;

  if (showLed == true)
    FastLED.show();
}

/* Switch between displaying LED Board and LED Tree */
void displayLEDs(bool showLed)
{
    displayBigBoardTwoTwo(showLed);
//    displayLEDTree(showLed);
//  #ifdef TEN_X_TWENTY
//    displayLEDBoardTwentyTen(showLed);
//  #endif
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
      bigDispBoard[14+j][1] = 5; // Green
      
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
    displayLEDs(true);
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
  displayLEDs(true);
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

  displayLEDs(true);
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
      displayLEDs(true);
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

  displayLEDs(true);
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
  digitalWrite(RESET_SWITCH_OUT, HIGH);
  digitalWrite(POWER_SWITCH_OUT, HIGH);
  digitalWrite(MUSIC_PIN_TETRIS, LOW);

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
  digitalWrite(MUSIC_PIN_TETRIS, HIGH);
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
  bool playMusic = false;
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
      else if (moveDir == MOVE_SELECT)
      {
        playMusic = !playMusic;
        if (playMusic)
          digitalWrite(MUSIC_PIN_TETRIS, HIGH);
        else
          digitalWrite(MUSIC_PIN_TETRIS, LOW);
      }
    
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
      if (digitalRead(RESET_SWITCH_IN) == HIGH)
        gameEnd = true;

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





/* SNAKE Function */



#define NUM_SNAKE_COLORS 11 
unsigned int snake_dir = MOVE_RIGHT;
unsigned int snake_leds[10] = {200,201,202,203,204,205,206,207,208,209}; // locations of 5 displayable snake leds, starting with head
unsigned int snake_color_idx = 0;
void play_snake()
{ 
  int new_snake_head_pos = 0;
  int i = 0;
  byte move_dir = 0;

  CRGB numToSnakeColor[NUM_SNAKE_COLORS] = 
 {CRGB::Blue, CRGB::Orange, CRGB::Yellow, CRGB::Red, CRGB::Green, CRGB::Cyan, CRGB::PeachPuff,
  CRGB::Purple, 0xD7FF00, CRGB::LightBlue, CRGB::White};

  delay(20);
    
    /* look for movements */
    move_dir = getMove();
    //if ((move_dir == MOVE_START) || (move_dir == MOVE_SELECT))
    //  snake_over = true;
    //else 
    if ((move_dir == MOVE_RIGHT) || (move_dir == MOVE_LEFT))
      snake_dir = move_dir;

    move_dir = move_dir >> 4;
    if (move_dir == MOVE_ROTATE_RIGHT)
      snake_color_idx = (snake_color_idx + NUM_SNAKE_COLORS + 1) % NUM_SNAKE_COLORS;
    else if (move_dir == MOVE_ROTATE_LEFT)
      snake_color_idx = (snake_color_idx + NUM_SNAKE_COLORS - 1) % NUM_SNAKE_COLORS;

    // update snake position
    if (snake_dir == MOVE_RIGHT)
      new_snake_head_pos = snake_leds[0] - 1;
    else if (snake_dir == MOVE_LEFT)
      new_snake_head_pos = snake_leds[0] + 1;
      
    if (new_snake_head_pos < 0)
      new_snake_head_pos = NUM_LEDS - 1;
    else if (new_snake_head_pos >= NUM_LEDS)
      new_snake_head_pos = 0;
    

    // Update snake tail
    for(i = 9; i > 0; i--)
      snake_leds[i] = snake_leds[i-1];
    snake_leds[0] = new_snake_head_pos;
     
    // Display snake
    for(i = 0; i < NUM_LEDS; i++)
    {
      if ((i == snake_leds[0]) || (i == snake_leds[1]) || (i == snake_leds[2]) || (i == snake_leds[3]) || (i == snake_leds[4]) || (i == snake_leds[5]) || (i == snake_leds[6]) || (i == snake_leds[7]) || (i == snake_leds[8]) || (i == snake_leds[9]))
        leds[i] = numToSnakeColor[snake_color_idx];
      else
        leds[i] = CRGB::Black;
    }
    for(i = 0; i < NUM_TREE_LEDS; i++)
    {
      if ((i == snake_leds[0]) || (i == snake_leds[1]) || (i == snake_leds[2]) || (i == snake_leds[3]) || (i == snake_leds[4]) || (i == snake_leds[5]) || (i == snake_leds[6]) || (i == snake_leds[7]) || (i == snake_leds[8]) || (i == snake_leds[9]))
        leds_tree[i] = numToSnakeColor[snake_color_idx];
      else
        leds_tree[i] = CRGB::Black;
    }
    FastLED.show();
    delay(20);
}



/* Lights Functions */



unsigned int light_twinkle = 0;
void display_green_red()
{
  int i;
  for(i = 0; i < NUM_LEDS; i++)
  {
    /* LEDS snake from 1 row to next */
    if (((i + light_twinkle / 100) % 3) == 0)
      leds[i] = CRGB::Red;
    else if (((i + light_twinkle / 100) % 3) == 1)
      leds[i] = CRGB::Green;
    else
      leds[i] = CRGB::White;
  }
  for(i = 0; i < NUM_TREE_LEDS; i++)
  {
    /* LEDS snake from 1 row to next */
    if (((i + light_twinkle / 100) % 3) == 0)
      leds_tree[i] = CRGB::Red;
    else if (((i + light_twinkle / 100) % 3) == 1)
      leds_tree[i] = CRGB::Green;
    else
      leds_tree[i] = CRGB::White;
  }
  
  if (light_twinkle < 300)
    light_twinkle++;
  else
    light_twinkle = 0;

  FastLED.show();
  delay(20);
}

uint8_t gHue = 0;
void display_rainbow()
{

  fill_rainbow( leds, NUM_LEDS, gHue, 7); 
  fill_rainbow( leds_tree, NUM_TREE_LEDS, gHue, 7); 
  if( random8() < CHANCE_OF_TWINKLE) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
    leds_tree[ random16(NUM_TREE_LEDS) ] += CRGB::White;
  }
     
  FastLED.show();
  delay(500);
  gHue++; // slowly cycle the "base color" through the rainbow
}


void display_lights()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  fadeToBlackBy( leds_tree, NUM_TREE_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
  pos = random16(NUM_TREE_LEDS);
  leds_tree[pos] += CHSV( gHue + random8(64), 200, 255);
  
  gHue++;
  FastLED.show();
  delay(200);
}

void display_one_color()
{
  for(int i = 0; i < NUM_LEDS; i++)
    leds[i] = CHSV( gHue, 200, 255);
  for(int i = 0; i < NUM_TREE_LEDS; i++)
    leds_tree[i] = CHSV( gHue, 200, 255);
  gHue++;
  FastLED.show();
  delay(200);
}

void display_sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  fadeToBlackBy( leds_tree, NUM_TREE_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
  pos = beatsin16( 13, 0, NUM_TREE_LEDS-1 );
  leds_tree[pos] += CHSV( gHue, 255, 192);
  FastLED.show();
  delay(50);
}

void display_juggle()
{
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  fadeToBlackBy( leds_tree, NUM_TREE_LEDS, 20);
  byte dothue = 0;
  
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    leds_tree[beatsin16( i+7, 0, NUM_TREE_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }

  FastLED.show();
  delay(50);
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
  fadeToBlackBy( leds, NUM_LEDS, 5);
  fadeToBlackBy( leds_tree, NUM_TREE_LEDS, 5);
  if( random8() < CHANCE_OF_TWINKLE) 
  {
    pos = random16(NUM_LEDS);
    leds[pos] += CHSV( 150, 200, 255);
    pos = random16(NUM_TREE_LEDS);
    leds_tree[pos] += CHSV( 150, 200, 255);
  }
  if( random8() < CHANCE_OF_TWINKLE) 
  {
    pos = random16(NUM_LEDS);
    leds[pos] += CRGB::White;
    pos = random16(NUM_TREE_LEDS);
    leds_tree[pos] += CRGB::White;
  }
  
  for(i = 0; i < NUM_DISP_ROWS; i++)
    for(j = 0; j < NUM_DISP_COLS; j++)
      bigDispBoard[i][j] = DISP_COLOR_BLACK;

  
  for(i = 0; i < 12; i++)
    for(j = 0; j < NUM_DISP_COLS_TETRIS; j++)
      bigDispBoard[i + unused_rows_top + 8][j + unused_cols_left] = pgm_read_byte_near(&castleDisp[i][j]);

  displayLEDs(false);


  FastLED.show();
  delay(40);
}



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
 {10,13,13,13,13,13,10,10,10,13,13,13,13,13,10,10,10}  // 16
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
 {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10}  // 16
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
 };


/* Only called in 22x22 mode at the moment */
/* Frames numbered 1-6 */
/* disp_mario_luigi: 0 - Mario, 1 - Luigi */
void displayLEDBoardMarioRun(unsigned char mario_frame, unsigned char mario_dir, unsigned char disp_mario_luigi)
{
  unsigned int i, j;
  unsigned int unused_cols_left = 2;
  unsigned int unused_rows_top = 2;
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
        
      if (mario_frame == 1)
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

  displayLEDs(true);
}

#define MARIO_DELAY_TIME 100

unsigned char run_move_dir = MOVE_NONE;
unsigned char disp_mario_luigi = 0; // 0 - Mario, 1 - Luigi
unsigned char mario_dir = MOVE_RIGHT;
boolean mario_move = false; // if moving this iteration
unsigned char mario_frame = 1; // 1-6: 1-Stand, 2-Run1, 3-Run2, 4-Run3, 5-Skid, 6-Jump
unsigned char mario_run_timer = 0; // TODO: Provide description
unsigned int delay_time = 100;
bool display_mario_run()
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
      disp_mario_luigi = (disp_mario_luigi + 1) % 2;
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

    displayLEDBoardMarioRun(mario_frame, mario_dir, disp_mario_luigi);
    delay(20);
  }

  return (disp_mario_luigi == 1);
}






void disp_mario_back()
{
  unsigned char i,j;
  
  for(i = 0; i < NUM_DISP_ROWS; i++)
  {
    for(j = 0; j < NUM_DISP_COLS; j++)
    {
      if (i < NUM_DISP_ROWS - 1)
        bigDispBoard[i][j] = DISP_COLOR_LIGHT_BLUE;
      else
        bigDispBoard[i][j] = DISP_COLOR_HALF_RED;
    }
  }
  
}


#define MARIO_FACE_COLOR DISP_COLOR_PEACH
#define MARIO_HAT_COLOR DISP_COLOR_RED
#define LUIGI_HAT_COLOR DISP_COLOR_GREEN
#define MARIO_PANTS_COLOR DISP_COLOR_BLUE
#define MARIO_SHOE_COLOR DISP_COLOR_BROWN
#define MARIO_FIRE_HAT_COLOR DISP_COLOR_WHITE
#define MARIO_FIRE_PANTS_COLOR DISP_COLOR_RED

unsigned char current_mario_row = NUM_DISP_ROWS - 2; /* bottom left of mario */
unsigned int current_mario_col = 3; /* (0 - 438), bottom left of mario */
unsigned int current_display_col = 0; /* leftmost column of display as level column */

bool mario_face_right = true;
bool mario_is_big = false;
bool mario_is_fire = false;
bool mario_is_luigi = false;
char current_mario_speed = 0; /* (-4,4) */
unsigned int mario_time = 0;
unsigned int mario_run_time = 0; /* time when mario most recently started running */


void disp_mario()
{
  unsigned char mario_col_now = current_mario_col - current_display_col;

  if (mario_is_big == false)
  {
    if (mario_face_right == true)
    {
      bigDispBoard[current_mario_row - 1][mario_col_now] = MARIO_HAT_COLOR; // top left pixel
      bigDispBoard[current_mario_row - 1][mario_col_now + 1] = MARIO_FACE_COLOR; // top right pixel
      /* blink front shoe depending on speed */
      if (current_mario_speed == 0)
        bigDispBoard[current_mario_row][mario_col_now + 1] = MARIO_PANTS_COLOR; // bottom right pixel
      else if ((mario_time % (8 / current_mario_speed)) < (4 / current_mario_speed))
        bigDispBoard[current_mario_row][mario_col_now + 1] = MARIO_PANTS_COLOR; // bottom right pixel
      else
        bigDispBoard[current_mario_row][mario_col_now + 1] = MARIO_SHOE_COLOR; // bottom right pixel
      bigDispBoard[current_mario_row][mario_col_now] = MARIO_PANTS_COLOR; // bottom left pixel
    }
    else
    {
      bigDispBoard[current_mario_row - 1][mario_col_now] = MARIO_FACE_COLOR; // top left pixel
      bigDispBoard[current_mario_row - 1][mario_col_now + 1] = MARIO_HAT_COLOR; // top right pixel
      /* blink front shoe depending on speed */
      if (current_mario_speed == 0)
        bigDispBoard[current_mario_row][mario_col_now] = MARIO_PANTS_COLOR; // bottom left pixel
      else if ((mario_time % (8 / current_mario_speed)) < (4 / current_mario_speed))
        bigDispBoard[current_mario_row][mario_col_now] = MARIO_PANTS_COLOR; // bottom left pixel
      else
        bigDispBoard[current_mario_row][mario_col_now] = MARIO_SHOE_COLOR; // bottom left pixel
      bigDispBoard[current_mario_row][mario_col_now + 1] = MARIO_PANTS_COLOR; // bottom right pixel
    }

  }
}

void play_mario(bool mario_is_green)
{
  bool mario_over = false;
  unsigned int move_dir = MOVE_NONE;

  mario_is_luigi = mario_is_green;

  // while (mario_over == false)
  {
    /* look for movements */
    move_dir = getMove();

    if ((move_dir & 0xF) == MOVE_RIGHT)
    {
      
    }

  }


  disp_mario_back();
  disp_mario();
  
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
#define BIG_DOT_TIME_EFFECT_GHOST 100

#define GHOST_TWO_TIME_START 100 /* BIG_DOT_TIME_EFFECT_GHOST + 30 */
#define GHOST_THREE_TIME_START 130 /* BIG_DOT_TIME_EFFECT_GHOST + 60 */

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

  displayLEDs(true);
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
        dot_color = DISP_COLOR_WHITE;
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
  if ((pac_counter % 1000) < 250)
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
  //timer_50 = pac_counter % 8 == 0; // 12.5 Hz -> (mod 8) == 0
  //timer_60 = pac_counter % 7 == 0; // 15 Hz -> (mod 7) == 0
  //timer_70 = (pac_counter % 6 == 0) || (pac_counter % 100) == 1; // 17.5 Hz -> (mod 6) == 0 || (mod 100) == 1
  //timer_80 = pac_counter % 5 == 0; // 20 Hz -> (mod 5) == 0
  //timer_90 = (pac_counter % 5 == 0) || (pac_counter % 40 == 19); // 22.5 Hz ((mod 5) == 0 || (mod 40) == 19 
  //timer_100 = pac_counter % 4 == 0; // 25 Hz -> (mod 4) == 0
  timer_50 = pac_counter % 8 == 0; // 12.5 Hz -> 100 (mod 8) == 0
  timer_60 = pac_counter % 7 == 0; // 15 Hz -> 100 (mod 7) == 0
  timer_70 = pac_counter % 6 == 0; // 17.5 Hz -> 100 (mod 6) == 0 || 100 (mod 100) == 1
  timer_80 = pac_counter % 5 == 0; // 20 Hz -> 100 (mod 5) == 0
  timer_90 = pac_counter % 4 == 0; // 22.5 Hz (100 (mod 5) == 0 || 100 (mod 40) == 19 
  timer_100 = pac_counter % 3 == 0; // 25 Hz -> 100 (mod 4) == 0

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
  bool play_music = false;

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
    else if (move_dir == MOVE_SELECT)
    {
      play_music = !play_music;
      if (play_music)
        digitalWrite(MUSIC_PIN_PAC, HIGH);
      else
        digitalWrite(MUSIC_PIN_PAC, LOW);
    }

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
    displayLEDs(true);

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

    
    delay(5);
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

  displayLEDs(true);

}



void start_menu()
{
  const unsigned int unused_rows_top = (NUM_DISP_ROWS - NUM_DISP_ROWS_MENU) / 2;
  const unsigned int unused_cols_left = (NUM_DISP_COLS - NUM_DISP_COLS_MENU) / 2;
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
    bool mario_is_green = display_mario_run();
    play_mario(mario_is_green);
  }
  delay(300);
  
}



/* Convert from bigDispBoard to LEDs on Tree */
/* Start at bottom, right and wind around tree */
void displayLEDTreeCal()
{
  int i, j;
  unsigned int currentLED = 0;
  
  /* bottom row to top, right to left */
 /*  for(i = num_tree_rows - 1; i >= 0; i--)
  {
    
   for(j = ledsBeforeRows[i + (NUM_DISP_ROWS_TETRIS - num_rows)] - 1; j >= 0; j--)
    {      
      leds_tree[currentLED] = CRGB::Black;
      currentLED++;
    }
    for(j = NUM_DISP_COLS_TETRIS - 1; j >= 0; j--)
    {      
      leds_tree[currentLED] = numToColor[bigDispBoard[i + unused_rows_top][j + unused_cols_left]];
      currentLED++;
    }
  }
  for(i = currentLED; i < NUM_TREE_LEDS; i++)
    if (showLed == true)
        leds_tree[currentLED] = CRGB::Black;
*/

}

/* Calibration Num Rows Display */
const unsigned char PROGMEM calNumRowsDisp[11][NUM_DISP_COLS_TETRIS] =
  {{5,5,5,4,0,4,5,0,5,0},
   {5,0,5,4,0,4,5,5,5,0},
   {5,0,5,4,0,4,5,0,5,0},
   {5,0,5,4,0,4,5,0,5,0},
   {5,0,5,4,4,4,5,0,5,0},
   {0,0,0,0,0,0,0,0,0,0},
   {5,5,5,4,4,4,5,0,5,0},
   {5,0,5,4,0,4,5,0,5,0},
   {5,5,0,4,0,4,5,0,5,0},
   {5,5,5,4,0,4,5,5,5,0},
   {5,0,5,4,4,4,5,0,5,0}};

/* Calibration Num Cols Display */
const unsigned char PROGMEM calNumColsDisp[11][NUM_DISP_COLS_TETRIS] =
  {{5,5,5,4,0,4,5,0,5,0},
   {5,0,5,4,0,4,5,5,5,0},
   {5,0,5,4,0,4,5,0,5,0},
   {5,0,5,4,0,4,5,0,5,0},
   {5,0,5,4,4,4,5,0,5,0},
   {0,0,0,0,0,0,0,0,0,0},
   {5,5,5,4,4,4,5,0,0,0},
   {5,0,0,4,0,4,5,0,0,0},
   {5,0,0,4,0,4,5,0,0,0},
   {5,0,0,4,0,4,5,0,0,0},
   {5,5,5,4,4,4,5,5,5,0}};

void dispCalNumRowsCols(bool isRows) /* isRows=true => Rows, else Cols */
{
  /* Use Tetris Playfield to display Pac Man Start screen */
  unsigned int unused_rows_top = (NUM_DISP_ROWS - NUM_DISP_ROWS_TETRIS) / 2;
  unsigned int unused_cols_left = (NUM_DISP_COLS - NUM_DISP_COLS_TETRIS) / 2;
  unsigned char i,j;
    
  for(i = 0; i < 11; i++)
    for(j = 0; j < NUM_DISP_COLS_TETRIS; j++)
    { 
      if (isRows == true)
        bigDispBoard[i + unused_rows_top][j + unused_cols_left] = pgm_read_byte_near(&calNumRowsDisp[i][j]);
      else
        bigDispBoard[i + unused_rows_top][j + unused_cols_left] = pgm_read_byte_near(&calNumColsDisp[i][j]);
    }

  displayBigBoardTwoTwo(true);
}

/* Calibration Num Cols Display */
const unsigned char PROGMEM calRowOffset[5][NUM_DISP_COLS_TETRIS] =
  {{5,5,5,4,4,4,5,0,5,0},
   {5,0,5,4,0,4,5,0,5,0},
   {5,5,0,4,0,4,5,0,5,0},
   {5,5,5,4,0,4,5,5,5,0},
   {5,0,5,4,4,4,5,0,5,0}};

void dispLedsCal(unsigned char row_num)
{
  unsigned char i,j;
  unsigned int unused_rows_top = (NUM_DISP_ROWS - NUM_DISP_ROWS_TETRIS) / 2;
  unsigned int unused_cols_left = (NUM_DISP_COLS - NUM_DISP_COLS_TETRIS) / 2;
  unsigned int tens_digit = (row_num % 100) / 10;
  unsigned int ones_digit = row_num % 10;
    
  for(i = 0; i < 5; i++)
    for(j = 0; j < NUM_DISP_COLS_TETRIS; j++)
      bigDispBoard[i + unused_rows_top][j + unused_cols_left] = pgm_read_byte_near(&calRowOffset[i][j]);

  /* Display row number */
  /* Display Score (number of lines) */
  for(j = 0; j < 5; j++) // for each column
  {
    if (((pgm_read_byte(&left_column[tens_digit]) >> j) & 0x1) > 0)
      bigDispBoard[7+j][9] = 4; // Red
    if (((pgm_read_byte(&middle_column[tens_digit]) >> j) & 0x1) > 0)
      bigDispBoard[7+j][10] = 4; // Red
    if (((pgm_read_byte(&right_column[tens_digit]) >> j) & 0x1) > 0)
      bigDispBoard[7+j][11] = 4; // Red
    if (((pgm_read_byte(&left_column[ones_digit]) >> j) & 0x1) > 0)
      bigDispBoard[7+j][13] = 5; // Green
    if (((pgm_read_byte(&middle_column[ones_digit]) >> j) & 0x1) > 0)
      bigDispBoard[7+j][14] = 5; // Green
    if (((pgm_read_byte(&right_column[ones_digit]) >> j) & 0x1) > 0)
      bigDispBoard[7+j][15] = 5; // Green 
  }

  displayBigBoardTwoTwo(true);
}




void calibrate_tree()
{
  bool cal_complete = false;
  unsigned int move_dir = MOVE_NONE;
  unsigned char i, j, k;
  unsigned char num_leds;
  
  /* First, determine number of display rows */
  while (cal_complete == false)
  {
    move_dir = getMove();

    /* range from 10 - NUM_DISP_ROWS_TREE */
    if (move_dir == MOVE_UP)
      num_tree_rows = ((num_tree_rows + 2) % (NUM_DISP_ROWS_TREE + 2)) % (NUM_DISP_ROWS_TREE - 9) + 10;
    else if (move_dir == MOVE_DOWN)
      num_tree_rows = ((num_tree_rows + NUM_DISP_ROWS - 1) % (NUM_DISP_ROWS_TREE + 1)) % (NUM_DISP_ROWS_TREE - 9) + 10;
    else if (move_dir == MOVE_START)
      cal_complete = true;

    for(j = 0; j < NUM_DISP_ROWS; j++)
        for(k = 0; k < NUM_DISP_COLS; k++)
        bigDispBoard[j][k] = DISP_COLOR_BLACK;
        
    /* Display Num Rows (stored in EEPROM) */
    displayScore(num_tree_rows, false);
    /* Re-display background */
    dispCalNumRowsCols(true);
    

    delay(80);
  }
  /* write num_rows to EEPROM */
  EEPROM.write(NUM_ROWS_EEPROM_ADDRESS, num_tree_rows);
  delay(250);
  cal_complete = false;

  /* Next, determine number of display columns */
  while (cal_complete == false)
  {
    move_dir = getMove();

   /* range from 10 - NUM_DISP_ROWS_TREE */
    if (move_dir == MOVE_UP)
      num_tree_cols = ((num_tree_cols + 2) % (NUM_DISP_COLS_TREE + 2)) % (NUM_DISP_COLS_TREE - 9) + 10;
    else if (move_dir == MOVE_DOWN)
      num_tree_cols = ((num_tree_cols + NUM_DISP_COLS - 1) % (NUM_DISP_COLS_TREE + 1)) % (NUM_DISP_COLS_TREE - 9) + 10;
    else if (move_dir == MOVE_START)
      cal_complete = true;

    for(j = 0; j < NUM_DISP_ROWS; j++)
        for(k = 0; k < NUM_DISP_COLS; k++)
        bigDispBoard[j][k] = DISP_COLOR_BLACK;

    /* Display Num Cols (stored in EEPROM) */
    displayScore(num_tree_cols, false);
    /* Re-display background */
    dispCalNumRowsCols(false);
    

    delay(80);
  }
  /* write num_cols to EEPROM */
  EEPROM.write(NUM_COLS_EEPROM_ADDRESS, num_tree_cols);
  delay(250);
  cal_complete = false;

  /* Finally get offsets for each row (number of down-spiral leds from end of last row */
  for(i = 0; i < num_tree_rows; i++)
  {
    num_leds = EEPROM.read(ROW_OFFSET_EEPROM_ADDRESS + i);
    cal_complete = false;
    while (cal_complete == false)
    {
      move_dir = getMove();

      if (move_dir == MOVE_UP)
        num_leds = (num_leds + 1) % MAX_LED_OFFSET;
      else if (move_dir == MOVE_DOWN)
        num_leds = (num_leds + MAX_LED_OFFSET - 1) % MAX_LED_OFFSET;
      else if (((i % 2) == 1) && (move_dir == MOVE_SELECT))
        cal_complete = true;

      for(j = 0; j < NUM_DISP_ROWS; j++)
        for(k = 0; k < NUM_DISP_COLS; k++)
        bigDispBoard[j][k] = DISP_COLOR_BLACK;

      /* Display Num Cols (stored in EEPROM) */
      displayScore(num_leds, false);
      /* Re-display background */
      dispLedsCal(i);

      delay(80);
    } /* end while loop for this row */
    /* Write EEPROM led offset value for this row */
    EEPROM.write(ROW_OFFSET_EEPROM_ADDRESS + i, num_leds);
    delay(250);
  }
  
}

/* Main Function */


unsigned char display_mode = DISP_LIGHT;
unsigned char calibration_mode = 0;
void loop() {
  unsigned int moveDir = MOVE_NONE;
  unsigned char i;
  unsigned char num_leds = 0;
  digitalWrite(MUSIC_PIN_TETRIS, LOW);
  digitalWrite(MUSIC_PIN_PAC, LOW);
  digitalWrite(MUSIC_PIN_MARIO, LOW);
  
  /* Monitor Inputs */
  moveDir = getMove();

  /* Move up and down between display options */
  if (moveDir == MOVE_UP)
    display_mode = (display_mode + NUM_DISP_MODES - 1) % NUM_DISP_MODES;
  else if (moveDir == MOVE_DOWN)
    display_mode = (display_mode + 1) % NUM_DISP_MODES;

  /* Update display for current selection */
  /* If tetris, init and kick out of loop */
  if ((moveDir == MOVE_START) || (digitalRead(RESET_SWITCH_IN) == HIGH))
    start_menu();
  else if (moveDir == MOVE_SELECT) /* Calibrate if SELECT, A, B pressed in order */
    calibration_mode = 1;
  else if ((moveDir >> 4 == MOVE_ROTATE_RIGHT) && (calibration_mode == 1))
    calibration_mode++;
  else if ((moveDir >> 4 == MOVE_ROTATE_LEFT) && (calibration_mode == 2))
  {
    calibrate_tree();
    calibration_mode = 0;
  }
  else if (display_mode == DISP_GR)
    display_green_red();
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
  else if (display_mode == DISP_SNAKE)
    play_snake();
    
  delay(50);

}
