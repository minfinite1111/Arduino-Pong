//Implementation of Pong on a 16x2 LCD screen by Manuel F. Moreno
//11.10.2018
#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

//piezo buzzer notes
#define NOTE_C4  262
#define NOTE_G3  196
#define NOTE_A3  220  
#define NOTE_B3  247
#define NOTE_A4  440
#define NOTE_E6  1319

//Array of musical notes that make up the winning melody: 
int melody[] = {NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
//simple array that holds the duration of each note.
int noteDurations[] = {4, 8, 8, 4, 4, 4, 4, 4};

int beepDuration = 5000;

//declaring variables for the ball 
byte gameBoard[16][80] = {};
int ballUpdateTime = 40;
byte ballXDir = 0;
byte ballYDir = -1;
byte ballX = 7;
byte ballY = 35;
byte ballCharArray[8] = {};
byte playerScore = 0;
byte aiScore = 0;
byte aiPaddlePos = 6;
byte myPaddlePos = 6;

// Holds paddle data in 2 separate arrays for easy 
//character printing
byte aiPaddleArray[2][8] = {0,0,0,0,0,1,1,1};
byte myPaddleArray[2][8] = {0,0,0,0,0,16,16,16};

// Used to make the logic easier when moving paddles
byte aiPaddleColArray[16] = {0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0};
byte myPaddleColArray[16] = {0,0,0,0,0,16,16,16,0,0,0,0,0,0,0,0};

// Potentiometers that move the paddles
const int moveRightButton = A2;
const int moveLeftButton = A1;

//Right and left potentiometer values 
int rightButState = 0;
int leftButState = 0;
int rightLastButState = 0;
int leftLastButState = 0;

//Prints 0 to each value in the paddle arrays. 
void ClearPaddles(){
  for(int i = 0; i < 2; i++){
    for(int j = 0; j < 8; j++){
      aiPaddleArray[i][j] = 0;
      myPaddleArray[i][j] = 0;
    }
  }
}//end of ClearPaddles
 
void PrintStuff(){
  //clear the screen
  lcd.clear();

  //Grab the character array stored in myPaddleArray using a pointer
  byte* mPTemp1 = myPaddleArray[0];
  byte* mPTemp2 = myPaddleArray[1];
  byte* aiPTemp1 = aiPaddleArray[0];
  byte* aiPTemp2 = aiPaddleArray[1];

  // Each character must have a unique character ID and array to print
  lcd.createChar(0, mPTemp1);
  lcd.createChar(1, mPTemp2);
  lcd.createChar(14, aiPTemp1);
  lcd.createChar(15, aiPTemp2);
  
  // Move cursor to 13 column on the 1st row
  lcd.setCursor(14, 0);

  // Draw the characters
  lcd.write(byte(0));
  lcd.setCursor(14, 1);
  lcd.write(byte(1));
  lcd.setCursor(1, 0);
  lcd.write(byte(14));
  lcd.setCursor(1, 1);
  lcd.write(byte(15));
 
  byte charNum = 0;
  
  if((ballY > 9) && (ballY < 70)){
    byte LCDCol = ballY / 5;
    byte LCDRow = (ballX <= 7) ? 0 : 1;
    charNum = ballY / 5;
    lcd.createChar(charNum, ballCharArray);
    lcd.setCursor(LCDCol,LCDRow);
    lcd.write(byte(charNum));
  }
}

//Prints everything to the screen.  
void PrintPaddles(){
  byte* mPTemp1 = myPaddleArray[0];
  byte* mPTemp2 = myPaddleArray[1];
  byte* aiPTemp1 = aiPaddleArray[0];
  byte* aiPTemp2 = aiPaddleArray[1];
  lcd.createChar(0, mPTemp1);
  lcd.createChar(1, mPTemp2);
  lcd.createChar(14, aiPTemp1);
  lcd.createChar(15, aiPTemp2);
  lcd.setCursor(14, 0);
  lcd.write(byte(0));
  lcd.setCursor(14, 1);
  lcd.write(byte(1));
  lcd.setCursor(1, 0);
  lcd.write(byte(14));
  lcd.setCursor(1, 1);
  lcd.write(byte(15));
}

//Used in the setup() to clear the screen and set up paddles. 
void SetupPaddles(){
  ClearPaddles();
  myPaddleArray[0][5] = 16;
  myPaddleArray[0][6] = 16;
  myPaddleArray[0][7] = 16;
  aiPaddleArray[0][5] = 1;
  aiPaddleArray[0][6] = 1;
  aiPaddleArray[0][7] = 1;
  PrintPaddles();
}

//Moves the paddle connected to A2 analogIn up 
void MovePaddleUp(){
  // Make sure the paddle can't go off the board
  if(myPaddlePos != 1){

    // Decrementing the paddle moves it up and update paddle A2 
    myPaddlePos--;
    UpdatePaddlesAfterMove();
  }
}

//Moves the paddle connected to A1 analogIn up
void MoveLeftPaddleUp(){
 
  // Make sure the paddle can't go off the board
  if(aiPaddlePos != 1){
 
    // Decrementing the paddle moves it up
    aiPaddlePos--;
    UpdatePaddlesAfterMoveLeftPlayer();
  }
}

//Moves the paddle connected to A2 analogIn up
void MovePaddleDown(){
  if(myPaddlePos != 14){
    myPaddlePos++;
    UpdatePaddlesAfterMove();
  }
}//right player

//Moves the paddle connected to A1 analogIn down
void MoveLeftPaddleDown(){
  if(aiPaddlePos != 14){
    aiPaddlePos++;
    UpdatePaddlesAfterMoveLeftPlayer();
  }
}//left player

//Right player paddle update.
void UpdatePaddlesAfterMove(){
  // Cycle through all bools turning on the next 1 above and off the 1 below
    for(int i = 0; i < 16; i++){
      if((i == (myPaddlePos-1)) || (i == myPaddlePos) || (i == (myPaddlePos+1))){
        myPaddleColArray[i] = 16;
      } else {
        myPaddleColArray[i] = 0;
      }
    }
 
    // Update the arrays used for printing the paddle characters
    for(int j = 0; j < 8; j++){
      myPaddleArray[0][j] = myPaddleColArray[j];
    }
    for(int k = 8; k < 16; k++){
      myPaddleArray[1][k-8] = myPaddleColArray[k];
    }
    
}

//Left player paddle update.
void UpdatePaddlesAfterMoveLeftPlayer(){
  //cycle through ai paddle(player 2)
        for(int i = 0; i < 16; i++){
      if((i == (aiPaddlePos-1)) || (i == aiPaddlePos) || (i == (aiPaddlePos+1))){
        aiPaddleColArray[i] = 1;
      } else {
        aiPaddleColArray[i] = 0;
      }
    }
 
    // Update the arrays used for printing the paddle characters
    for(int j = 0; j < 8; j++){
      aiPaddleArray[0][j] = aiPaddleColArray[j];
    }
    for(int k = 8; k < 16; k++){
      aiPaddleArray[1][k-8] = aiPaddleColArray[k];
    }
}

//function used to determine location of the ball on the gameboard. 
int GetLEDRowValue(byte ledRow, byte maxColumn){ 
  int minColumn = maxColumn - 4;
  int ledValue = 0; 
  int multiplier = 1;
  for(int i = maxColumn; i >= minColumn; i--){
    ledValue += (gameBoard[ledRow][i] * multiplier);
    multiplier *= 2; 
  }
  return ledValue;
}
  
void GenerateBallArray(){
 
  byte maxCol = ((ballY / 5) * 5) + 4;
  byte minCol = maxCol - 4;
  byte startRow = (ballX <= 7) ? 0 : 8;
  byte endRow = startRow + 8;
  if(startRow == 0){
    for(int i = startRow; i < endRow; i++){
      ballCharArray[i] = GetLEDRowValue(i, maxCol);
    }
  } else {
    for(int i = startRow; i < endRow; i++){
      ballCharArray[i-8] = GetLEDRowValue(i, maxCol);
    }
  }
}

void SetupBall(){
  ballYDir *= -1;
  gameBoard[7][35] = true;
}//sets the board up 


//Awards a point if the right or left player(left player is called "ai")
//score a point. 
void AwardAPoint(){
  if(ballY <= 8){
    playerScore++;
  } else {
    aiScore++;
  }
  delay(100);
  ballYDir *= -1;
}

//Makes the piezo buzzer beep when the on screen ball touches something. 
void BeepSound(){
tone(8, NOTE_C4, beepDuration);
delay(50);
noTone(8);
}

//When the right or left player score a point, a high pitch beep
//is made by the piezo. 
void BeepSoundPoint(){
  tone(8,NOTE_E6,beepDuration);
  delay(50);
  noTone(8);
}

//Plays a short melody on the piezo buzzer when either player wins.
//Melody created by Tom Igoe on the 21st of Jan 2010
//modified 30 Aug 2011
void PlayWinningSound(){
    // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 8; thisNote++) {

    // to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(8, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(8);
  }
}

//Checks which player has reached a score of 2 first. 
void CheckForWinner(){
  if(aiScore == 2){
    lcd.clear();
    lcd.print("Left Player ");
    lcd.setCursor(0, 1);
    lcd.print("wins! ");
    PlayWinningSound();
  }
  if(playerScore == 2){
    lcd.clear();
    lcd.print("Right Player ");
    lcd.setCursor(0, 1);
    lcd.print("wins! ");
    PlayWinningSound();
  }
}

//This function handles all possible ball collisions. 
//Calls the BeepSoundPoint and AwardAPoint functions when applicable. 
//Possible Collisons 
void UpdateBall(){
  delay(ballUpdateTime);
  if((ballY <= 8) || (ballY >= 71)){
    //Ball has made it beyond a paddle give player a point.
    ballYDir *= 1;
    AwardAPoint();
    BeepSoundPoint();
  } else if((ballX == 0) || (ballX == 15)){ 
    //Ball has hit the top or bottom of the screen.
    //change its direction so it doesn't go off screen.
    ballXDir *= -1;
    BeepSound();
  } else if((ballY == 69) && (ballX == myPaddlePos)){
    //Ball hits right players paddle in the middle
    ballYDir *= -1;
    BeepSound();
  } else if((ballY == 69) && (ballX == (myPaddlePos + 1))){
    //Ball hit the bottom of the players paddle.  
    ballYDir *= -1;
    ballXDir = 1;
    BeepSound();
  } else if((ballY == 69) && (ballX == (myPaddlePos - 1))){
    //Ball hit the top of the right players paddle.
    ballYDir *= -1;
    ballXDir = -1;
    BeepSound();
  } else if((ballY == 10) && (ballX == (aiPaddlePos))){
    //Ball hit the left(also called ai controls analogIn A2) players paddle
    //in the center. 
    ballYDir *= -1;
    ballXDir = -1;
    BeepSound();
  } else if((ballY == 10) && (ballX == (aiPaddlePos + 1))){
    //Ball hit the bottom of left players paddle. 
    ballYDir *= -1;
    ballXDir = -1;
    BeepSound();
  }else if((ballY == 10) && (ballX == (aiPaddlePos - 1))){
    //ball hit the top of the left players paddle. 
    ballYDir *= -1;
    ballXDir = -1;
    BeepSound();
  }
  // Deletes last ball position and adds a new ball position
  // to the gameboard.
  gameBoard[ballX][ballY] = false;
  ballX += ballXDir;
  ballY += ballYDir;
  gameBoard[ballX][ballY] = true;
  GenerateBallArray();
  PrintStuff();
}

//sets up the game on startup 
void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  pinMode(moveRightButton, INPUT);
  pinMode(moveLeftButton, INPUT);
  SetupBall();
  GenerateBallArray();
  SetupPaddles();
  PrintStuff();
}

//the main game loop
void loop() {
  UpdateBall();
  
// Get current button state
  //get current pententiometer value
  rightButState = analogRead(moveRightButton);
  leftButState = analogRead(moveLeftButton);
  
  // Check the button state change
  if(rightButState != rightLastButState){
 
    // Check if button was pressed
    if(rightButState > rightLastButState){
 
      // Move paddle up if you can and redraw
      MovePaddleUp();
      PrintStuff();
    }
    else if(rightButState < rightLastButState){
      MovePaddleDown();
      PrintStuff();
    }
    // Added to avoid bouncing
    delay(50);
 
    // Store last button state
    rightLastButState = rightButState;
  }
 
  // Check the button state change
  if(leftButState != leftLastButState){
 
    // Check if button was pressed
    if(leftButState > leftLastButState){
 
      // Move paddle up if you can and redraw
      MoveLeftPaddleUp();
      PrintStuff();
    }
    else if(leftButState < leftLastButState){
      MoveLeftPaddleDown();
      PrintStuff();
    }
    // Added to avoid bouncing
    delay(50);
 
    // Store last button state
    leftLastButState = leftButState;
   
  }
   CheckForWinner();
}
