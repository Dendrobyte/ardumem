#include <Arduino.h>
#line 1 "/Users/markbacon/Documents/arduboy-simon/arduboy-simon.ino"
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
BeepPin1 beep;

boolean buttonA, up, right, left, down; // All combos

void showStartScreen();
void gameRunLoop();
void drawGameState();
void resetDrawnRectFor(int rectNum);
void playVisAndToneForSeq(int rectNum);

// TODO: Set up tones

#line 27 "/Users/markbacon/Documents/arduboy-simon/arduboy-simon.ino"
void setup();
#line 38 "/Users/markbacon/Documents/arduboy-simon/arduboy-simon.ino"
void loop();
#line 211 "/Users/markbacon/Documents/arduboy-simon/arduboy-simon.ino"
void drawHorizRect(int topLeftX, int topLeftY);
#line 215 "/Users/markbacon/Documents/arduboy-simon/arduboy-simon.ino"
void drawVertRect(int topLeftX, int topLeftY);
#line 219 "/Users/markbacon/Documents/arduboy-simon/arduboy-simon.ino"
void drawFilledHorizRect(int topLeftX, int topLeftY);
#line 223 "/Users/markbacon/Documents/arduboy-simon/arduboy-simon.ino"
void drawFilledVertRect(int topLeftX, int topLeftY);
#line 27 "/Users/markbacon/Documents/arduboy-simon/arduboy-simon.ino"
void setup() {
  arduboy.begin();
  beep.begin();
  arduboy.setFrameRate(30);
}

// Flags for which screen to render
// Fine for now vs enum
boolean startScreen = true;
boolean inGame = false;
boolean endScreen = false;
void loop() {
  if (!(arduboy.nextFrame())) return; // looks like it prevents async shit
  arduboy.clear(); // Maybe we don't always want to run this?
  beep.timer(); // doesn't stop without this, must be a manager

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
int seqLength = 0; // Avoid length call, and next seq item gets added here
int seqCurrPtr = -1; // Track current item being played
int seq[63]; // WR is 84, ggwp
boolean waitingForInput = false; // Don't listen for input when sequence is playing
boolean addNewNumToSeq = true;
char textBuffer[16]; // Have one allocated text buffer to reuse constantly
int selectedRect = 0; // 0/1/2/3/4 == none/up/down/left/right

// Sequence related state
boolean isPlayingSequence = false;

// Entire game loop, using booleans as state again
// Need ticks and timers for sequences and draws and whatnot
int tick = 0;
int seqTimerTickVal = 30; // make this a const, num of frames ig. loop is 1x per frame?
void gameRunLoop() {
  // Create a timer that waits to play first part of seq
  // If we're not waiting for input, it means we can trigger next part of sequence
  if (!waitingForInput and !isPlayingSequence) {
    // Find random num, add to seq, then play the sequence
    int newSeq = (rand() % (4)); // 0-3
    seq[seqLength] = newSeq;
    seqLength += 1;
    currLevel += 1;
    tick = seqTimerTickVal;
    isPlayingSequence = true;
  }
  
  // Use current tick and delta whatever to play entire sequence
  // I might just be making my own timer? Lol idk
  if (!waitingForInput and isPlayingSequence) {
    // If the tick is reset, play next sequence (or end)
    if (tick == 0) {
      seqCurrPtr += 1;
      if (seqCurrPtr == seqLength-1) {
        // End seq, allow input
        isPlayingSequence = false;
        waitingForInput = true;
      } else {
        // Set the empty rectangle to draw now
        int currRect = seq[seqCurrPtr];
        resetDrawnRectFor(currRect); // May actually be unnecessary if this draws after

        // First play for this
        //playVisAndToneForSeq(currRect);
        
        // Reset tick for the countdown
        tick = seqTimerTickVal;
      }
    } else {
      // between 10 -> 1, do nothing as a pause
      if (tick < 10) {
        // do nothing
      } else if (tick < 30) {
        // between 30 -> 11, play the next tone/show the next rect
        playVisAndToneForSeq(seq[seqCurrPtr]);
      }

      tick -= 1;
    }
  }

  // Finally if we're waiting for input, we check for right and wrong

  // Some test stuff
  if (arduboy.pressed(UP_BUTTON)) {
    selectedRect = 1;
  } else if (arduboy.pressed(DOWN_BUTTON)) {
    selectedRect = 2;
  } else if (arduboy.pressed(LEFT_BUTTON)) {
    selectedRect = 3;
  } else if (arduboy.pressed(RIGHT_BUTTON)) {
    selectedRect = 4;
  }
}

// Coordinates feel easier for fill and unfilled
int northRect[2] = {80, 7};
int southRect[2] = {80, 47};
int westRect[2] = {72, 15};
int eastRect[2] = {112, 15};
int* numToRectMap[4] = {northRect, southRect, westRect, eastRect};
int numToFreqMap[4] = {200, 261, 523, 175};

// Make it exist first, pretty later (/v.v)/
void playVisAndToneForSeq(int rectNum) {
  // Show corresponding filled rectangle
  int coord_x = numToRectMap[rectNum][0];
  int coord_y = numToRectMap[rectNum][1];
  if (rectNum == 0 || rectNum == 1) drawFilledHorizRect(coord_x, coord_y); 
  if (rectNum == 2 || rectNum == 3) drawFilledVertRect(coord_x, coord_y); 

  // Play corresponding tone
  beep.tone(beep.freq(numToFreqMap[rectNum]), 5);
}

// May not even need this since default draw is empty?
void resetDrawnRectFor(int rectNum) {
  int coord_x = numToRectMap[rectNum][0];
  int coord_y = numToRectMap[rectNum][1];
  if (rectNum == 0 || rectNum == 1) drawHorizRect(coord_x, coord_y); 
  if (rectNum == 2 || rectNum == 3) drawVertRect(coord_x, coord_y); 
}

// Draw the curr level, what seq to draw, etc;
// TODO: Gotta find the right spot for cursor and whatnot
// 128×64 are the screen dims
void drawGameState() {
  // Curr Level Text
  arduboy.setTextSize(1);
  arduboy.setCursor(8, 8);
  arduboy.print("Curr Level");
  arduboy.setCursor(24, 20);
  
  sprintf(textBuffer, "%2d", currLevel);
  arduboy.print(textBuffer);

  // Print the generic simon stuff (TOP, BOTTOM, LEFT, RIGHT)
  // NOTE: Starting with just boxes
  arduboy.setCursor(0, 0);
  drawHorizRect(northRect[0], northRect[1]);
  drawHorizRect(southRect[0], southRect[1]);
  drawVertRect(westRect[0], westRect[1]);
  drawVertRect(eastRect[0], eastRect[1]);

  if (selectedRect != 0) {
    if (selectedRect == 1) {
      drawFilledHorizRect(northRect[0], northRect[1]);
    }
    if (selectedRect == 2) {
      drawFilledHorizRect(southRect[0], southRect[1]);
    }
    if (selectedRect == 3) {
      drawFilledVertRect(westRect[0], westRect[1]);
    }
    if (selectedRect == 4) {
      drawFilledVertRect(eastRect[0], eastRect[1]);
    }
  }
}

int cornerRad = 0; // For now to test sizing
void drawHorizRect(int topLeftX, int topLeftY) {
  arduboy.drawRoundRect(topLeftX, topLeftY, 32, 8, cornerRad); 
}

void drawVertRect(int topLeftX, int topLeftY) {
  arduboy.drawRoundRect(topLeftX, topLeftY, 8, 32, cornerRad);
}

void drawFilledHorizRect(int topLeftX, int topLeftY) {
  arduboy.fillRoundRect(topLeftX, topLeftY, 32, 8, cornerRad); 
}

void drawFilledVertRect(int topLeftX, int topLeftY) {
  arduboy.fillRoundRect(topLeftX, topLeftY, 8, 32, cornerRad);
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

