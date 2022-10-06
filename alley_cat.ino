#include <LiquidCrystal.h>
#include <toneAC.h>

// Create an LCD object
// The Arduino pins are connected to the
// LCD pins in the following order:
// RS(4), EN(6), D4(11), D5(12), D6(13), D7(14)
LiquidCrystal lcd(11, 12, 4, 5, 6, 7);

const int lcdRows = 2;
const int lcdCols = 16;

// labels for obstacles and rewards
const int G = 6;
const int W = 7;
const int F = 8;
const int M = 8;

// cat's remaining lives
volatile int C = 5;

volatile bool isCrash = 0;
volatile bool catIndex = 0;
volatile unsigned int score = 0;
volatile bool buttonPush = 0;
 
byte kittyFull[] = {
 B01010,
 B11111,
 B10101,
 B11011,
 B01110,
 B11111,
 B01110,
 B11011,
};
byte kittyNoArms[] = {
 B01010,
 B11111,
 B10101,
 B11011,
 B01110,
 B01110,
 B01110,
 B11011,
};
byte kittyNoLeg[] = {
 B01010,
 B11111,
 B10101,
 B11011,
 B01110,
 B01110,
 B01110,
 B00011,
};
 
byte kittyBody[] = {
 B00000,
 B00000,
 B00000,
 B00000,
 B00000,
 B01111,
 B01111,
 B01111
};
byte kittyHead[] = {
 B00000,
 B00000,
 B00000,
 B01110,
 B11011,
 B10110,
 B11011,
 B01110
};
 
byte garbage[] = {
 B00100,
 B01110,
 B10001,
 B11111,
 B10101,
 B10101,
 B10101,
 B11111
};
byte wall[] = {
 B00011,
 B11111,
 B10001,
 B11111,
 B01001,
 B11111,
 B10000,
 B11111
};
byte mouse[] = {
 B00100,
 B01110,
 B10101,
 B11111,
 B11111,
 B01110,
 B00100,
 B00111
};
 
int screenStr[lcdRows][lcdCols] = 
{{' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '},
{ C ,' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '}};

void setup() {
 pinMode(2, INPUT);
 pinMode(3, OUTPUT);
 pinMode(13, OUTPUT);
 
 // Initialize a 16 x 2 display.
 lcd.begin(lcdCols,lcdRows);

 // Initialize custom icons
 lcd.createChar(C, kittyFull);
 lcd.createChar((C-1), kittyNoArms);
 lcd.createChar((C-2), kittyNoLeg);
 lcd.createChar(1, kittyBody);
 lcd.createChar((C-3), kittyHead);
 lcd.createChar(G, garbage);
 lcd.createChar(W, wall);
 lcd.createChar(M, mouse);
 
 attachInterrupt(0, buttonfcn, RISING);
}

void loop() {
 startGame();
 instructions();
 int gameSpeed = selectDifficulty(); 
 playGame(gameSpeed);
 endGame();
}
 
void printLCD(){
 lcd.clear();
 lcd.setCursor(0,0);
 
 for(int j = 0; j <  2 ; j++){
   for(int i = 0; i <= 15; i++){
     lcd.write(screenStr[j][i]);
   }
   lcd.setCursor(0,1);
 }
}
 
void startGame(){
 // Initialize variables
 buttonPush = 0;
 score = 0; 
 C = 5;
 catIndex = 0; 

 // Initialize empty board with cat
 for(int j = 0; j <  2 ; j++){
   for(int i = 0; i <= 15; i++){
     screenStr[j][i] = ' ';
   }
 }
 screenStr[catIndex][0] = C;

 // Write message
 lcd.clear(); 
 lcd.setCursor(0,0);
 lcd.write(C);
 lcd.write(" Alley Cat");
 lcd.setCursor(0,1);
 lcd.write("Click to start!");  

 while(buttonPush == 0);    
}
 
void instructions(){
 buttonPush = 0;

 // Write message
 lcd.clear(); 
 lcd.setCursor(0,0);
 lcd.write("Hit:   ");
 lcd.write(M);
 lcd.setCursor(0,1);
 lcd.write("Avoid: ");
 lcd.write(W);
 lcd.write(" ");
 lcd.write(G);
 
 while (buttonPush == 0);
}
 
int selectDifficulty(){
 int gameSpeed;
 int pot_read;
 buttonPush = 0;
 
 while (buttonPush == 0){
   // Write message
   lcd.clear();
   lcd.setCursor(0,0);
   lcd.write("Select level");
   lcd.setCursor(0,1);
   lcd.write("with knob: ");    

   // Read input from potentiometer
   pot_read = analogRead(A1);

   // Translate input to difficulty selection
   if (pot_read < 350){
     gameSpeed = 250;
     lcd.write("Turbo");
   } else if (pot_read < 700){
     gameSpeed = 500;
     lcd.write("Hard");    
   } else{
     gameSpeed = 1000;
     lcd.write("Easy");
   }

   // Collect input 10x/sec
   delay(100);
  }
  
  return gameSpeed;
}
 
void playGame(int gameSpeed){
 while (C >= 2){
  isCrash = false;
  updateScreen();
  digitalWrite(3, LOW);     // turn off LED
  delay(gameSpeed);
  if(isCrash)  toneAC(196, 10, 1000);     // play piezo tone
 }
}
 
void updateScreen(){
 randomSeed(micros()+analogRead(A0)+5);
 char newCharObst  = ' ';
 char newCharTreat = ' ';

 shiftScreen();
 
 bool kitty_dead = check_collision();
 if (kitty_dead) return;

 screenStr[catIndex][0] = C;

 // Pick an object to generate
 int val = random(100);
 if (val < 40) {
   // no new object
 } else if (val < 65) {
   newCharObst = G;
 } else if (val < 93) {
   newCharObst = W;
 } else {
   newCharTreat = M;
 }

 // Pick a place to generate the object
 if (screenStr[1][-2] != G && screenStr[1][-2] != W ){
   if (screenStr[2][-2] != G && screenStr[2][-2] != W ){
     val = random(2);
     screenStr[!(val)+1][-1] = newCharObst;
     screenStr[val+1][-1] = newCharTreat; 
   } else {
     // Obstacle in bottom row
     screenStr[2][-1] = newCharObst;
     screenStr[1][-1] = newCharTreat;
   }
 } else {
   // Obstacle in top row
   screenStr[1][-1] = newCharObst;
   screenStr[2][-1] = newCharTreat;
 }         

 printLCD();
}
 
void shiftScreen(){
 for (int j = 0; j <= lcdRows - 1; j++){
   for (int i = 0; i <= lcdCols - 2; i++){  
     screenStr[j][i] = screenStr[j][i+1];
   }
 }
}

bool check_collision(){
  bool kitty_dead = 0;

  if(screenStr[catIndex][0] == G || screenStr[catIndex][0] == W){     // collision with obstacle
   kitty_dead = crash();
 } else if(screenStr[catIndex][0] == F || screenStr[catIndex][0] == M){   // collision with treat
   point();
 }

 return kitty_dead;
}
 
bool crash(){
 C--;         // decrement kitty's lives remaining
 screenStr[catIndex][0] = C;
 isCrash = true;
 if (C < 2) return 1;     // kitty is out of lives - end game
 
 return 0;
}
 
void point(){
 score++;
 digitalWrite(3, HIGH); // turn on green LED
}
 
void endGame(){
 buttonPush = 0;

 // Write message
 lcd.clear();
 lcd.setCursor(0,0);
 lcd.write(1);
 lcd.write(2);
 lcd.write(" GAME OVER.");

 // Write score
 lcd.setCursor(0,1);
 lcd.write("Score: ");
 lcd.print((int)score);
 
 while (buttonPush == 0);
}
 
void buttonfcn() {
 // Debounce
 static unsigned long last_t = 0;
 unsigned long curr_t = millis();
 if (curr_t - last_t < 250) return;
 last_t = curr_t;
  
 buttonPush = 1;

 catIndex = (!catIndex); // switch cat's row position

 bool kitty_dead = check_collision();
 if (kitty_dead) return;

 screenStr[catIndex][0] = C;
 screenStr[!catIndex][0] = ' ';
 printLCD();
}
