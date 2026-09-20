/* 
Ardumem game designed for Arduboy2
by Dendrobyte

Feel free to distribute or whatever
*/

#include <Arduboy2.h>

// TODO: Use EEPROM(?) for save filesand whatnot
// Looks like this is a space allocation thing?

// Initial variable dec
Arduboy2 arduboy;

boolean buttonA, up, right, left, down; // All combos

// TODO: Set up tones

void setup() {
  arduboy.begin();
  arduboy.setFrameRate(30);
}

// Flags for which screen to render
// Fine for now vs enum
boolean startScreen = true;
boolean inGame = false;
boolean endScreen = false;
void loop() {
  arduboy.clear() // Maybe we don't always want to run this?
  // These could be a switch / screen state
  if (startScreen) {
    // Draw start screen
    showStartScreen();

    boolean startPressed = arduboy.pressed(A_BUTTON);
    if (startPressed) {
      startScreen = false;
      inGame = true;
      drawGameState(); // initial draw
    }
  } else if (inGame) {
    // Deciding to run a loop that updates gamestate, then something that draws everything
    gameRunLoop();
    drawGameState();
  } else if (endScreen) {
    // Do last, show score, etc.
    // Worth figuring out the cursor positions before I get too deep into this part
  }

  // Then you run display to show everything drawn
  arduboy.display();
}

// Game state
int currLevel = 0;
int currLvlTxt
int seqLength = 0; // to avoid a length call idk
int seqCurrPtr = 0; // Track where we are adding into the sequence
int[63] seq = {}; // WR is 84, ggwp
boolean allowInput = false; // Don't listen for input when sequence is playing
boolean addNewNumToSeq = true;
char textBuffer[16]; // Have one allocated text buffer to reuse constantly

// Entire game loop, using booleans as state again
int tick;
void gameRunLoop() {
  // Create a timer that waits to play first part of seq
  // This loop needs a tick imo
}

// Draw the curr level, what seq to draw, etc;
// TODO: Gotta find the right spot for cursor and whatnot
// 128×64 are the screen dims
void drawGameState() {
  // Curr Level Text
  arduboy.setTextSize(1);
  arduboy.setCursor(16, 22);
  arduboy.print("Curr Level");
  arduboy.setCursor(20, 24);
  
  sprintf(textBuffer, "%2d", currLevel);
  arduboy.print(textBuffer);

  // Print the generic simon stuff (TOP, BOTTOM, LEFT, RIGHT)
  // NOTE: Starting with just boxes
  drawHorizRect(80, 7);
  drawHorizRect(80, 48);
  drawVertRect(71, 16);
  drawVertRect(112, 16);
}

void drawHorizRect(int topLeftX, int topLeftY) {
  arduboy.drawRect(topLeftX, topLeftY, 16, -8); // assuming it's w then height? 
}

void drawVertRect(int topLeftX, int topLeftY) {
  arduboy.drawRect(topLeftX, topLeftY, 8, -16); // assuming it's w then height? 
}
// Draws the start screen stuff
void showStartScreen() {
  arduboy.setCursor(16, 22);
  arduboy.setTextSize(2);
  arduboy.print("ARDUMEM");
  arduboy.setTextSize(1);
  arduboy.setCursor(31, 53);
  arduboy.print("A TO START");
}
