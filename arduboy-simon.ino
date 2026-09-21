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

void setup() {
  arduboy.begin();
  beep.begin();
  arduboy.setFrameRate(30);
}

// Flags for which screen to render
// Fine for now vs enum
boolean startScreen = true;
boolean inGame = false;
boolean isGameOver = false;

void loop() {
  if (!(arduboy.nextFrame())) return; // looks like it prevents async shit
  arduboy.clear(); // Maybe we don't always want to run this?
  beep.timer(); // doesn't stop without this, must be a manager
  arduboy.pollButtons();

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
  } else if (isGameOver) {
    showEndScreen();
    // Do last, show score, etc.
    // Worth figuring out the cursor positions before I get too deep into this part
    boolean startPressed = arduboy.pressed(A_BUTTON);
    if (startPressed) {
      startScreen = false;
      inGame = true;
      isGameOver = false;
      // This needs to reset all the game state too
      drawGameState(); // initial draw
    }
  } else if (inGame) {
    // Deciding to run a loop that updates gamestate, then something that draws everything
    gameRunLoop();
    drawGameState();
  }

  // Then you run display to show everything drawn
  arduboy.display();
}

// Game state
int currLevel = 0;
int seqLength = 0; // Avoid length call, and next seq item gets added here
int seqCurrPtr; // Track current item being played
int seq[63]; // WR is 84, ggwp
boolean waitingForInput = false; // Don't listen for input when sequence is playing
boolean addNewNumToSeq = true;
char textBuffer[16]; // Have one allocated text buffer to reuse constantly

// Sequence related state
boolean isPlayingSequence = false;
boolean isBetweenRounds = false;

// Entire game loop, using booleans as state again
// Need ticks and timers for sequences and draws and whatnot
int tick = 0;
int seqTimerTickVal = 30; // make this a const, num of frames ig. loop is 1x per frame?

// Tracking the input state
// currGuess must equal seq[guessCtr] until the end
int guessCtr = 0;
int currGuess = -1;

// Run whatever and update state necessary
void gameRunLoop() {
  // Create a timer that waits to play first part of seq
  // If we're not waiting for input, it means we can trigger next part of sequence
  if (!waitingForInput && !isPlayingSequence) {
    newItemToSeq();
  }
  
  // Use current tick and delta whatever to play entire sequence
  // I might just be making my own timer? Lol idk
  else if (!waitingForInput && isPlayingSequence) {
    // Small pause between rounds
    // Foolishly relying on tick being 0 when this starts from newItemToSeq
    if (isBetweenRounds) {
      if (tick > 0) {
        tick--;
      } else {
        isBetweenRounds = false;
        tick = seqTimerTickVal;
      }
      return;
    }

    // If the tick is reset, play next sequence (or end)
    arduboy.setCursor(24, 32);
    
    sprintf(textBuffer, "%2d", seqCurrPtr);
    arduboy.print(textBuffer);
    if (tick == 0) {
      if (seqCurrPtr == seqLength-1) {
        // End seq, allow input
        isPlayingSequence = false;
        waitingForInput = true;
      } else {
        // Set the empty rectangle to draw now (i think it's unnecessary)
        //int currRect = seq[seqCurrPtr];
        //resetDrawnRectFor(currRect);

        // First play for this
        //playVisAndToneForSeq(currRect);
        
        // Reset tick for the countdown
        tick = seqTimerTickVal;
        seqCurrPtr++;
      }
    } else {
      // between 10 -> 1, do nothing as a pause
      if (tick < 10) {
        // do nothing
      } else if (tick <= seqTimerTickVal) {
        // between 30 -> 11, play the next tone/show the next rect
        arduboy.setCursor(24, 48);
        
        sprintf(textBuffer, "%2d", seqCurrPtr);
        arduboy.print(textBuffer);
        playVisAndToneForSeq(seq[seqCurrPtr]);
      }

      tick--;
    }
  }

  // Finally if we're waiting for input, we check for right and wrong
  // We don't need to track the whole seq, just if what's currently correct is good
  else if (waitingForInput) {
    // TODO: Timer to pause and continue playing the guess so it's not just a blip
    // idk if I can "get the pressed button" but either way it's a case situation
    if (arduboy.justPressed(UP_BUTTON)) {
      playVisAndToneForSeq(0);
      currGuess = 0;
    } else if (arduboy.justPressed(DOWN_BUTTON)) {
      playVisAndToneForSeq(1);
      currGuess = 1;
    } else if (arduboy.justPressed(LEFT_BUTTON)) {
      playVisAndToneForSeq(2);
      currGuess = 2;
    } else if (arduboy.justPressed(RIGHT_BUTTON)) {
      playVisAndToneForSeq(3);
      currGuess = 3;
    }
    
    // Is there a better way to wait (poll?) for a button press
    if (currGuess != -1) {
      if (currGuess != seq[guessCtr]) {
        // Failed, show score, etc.
        // For now just show end screen
        beep.tone(beep.freq(100), 5);
        isGameOver = true;
        return;
      }
      
      guessCtr++;
      // Check if that was the last guess, and thus move on
      if (guessCtr == seqLength) {
        // If we've now guessed the number of items in seq, good job move on
        guessCtr = 0;
        waitingForInput = false;
        isBetweenRounds = true;
      } // Else we do nothing. Guess ctr already progressed
       
      currGuess = -1;
    }
  }
}

void newItemToSeq() {
    // Find random num, add to seq, then play the sequence
    // TODO: Random seed bc I don't think it's random rn
    int newSeq = (rand() % (4)); // 0-3
    seq[seqLength] = newSeq;
    seqLength += 1;
    currLevel += 1;
    seqCurrPtr = 0;
    tick = seqTimerTickVal;
    isPlayingSequence = true;
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
  beep.tone(beep.freq(numToFreqMap[rectNum]), 2);
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

void showEndScreen() {
  arduboy.setCursor(16, 22);
  arduboy.setTextSize(2);
  arduboy.print("ARDUMEM");
  arduboy.setTextSize(1);
  arduboy.setCursor(31, 53);
  arduboy.print("A TO TRY AGAIN");
  
  // Lazy way of resetting game state
  // Anything not here just gets overwritten anyway
  currLevel = 0;
  seqLength = 0;
  waitingForInput = false;
  addNewNumToSeq = true;

  isPlayingSequence = false;
  isBetweenRounds = false;

  tick = 0;

  guessCtr = 0;
  currGuess = -1;

}
