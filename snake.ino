#include "LedControl.h"  //  need the library
#include <LiquidCrystal.h>
#include <EEPROM.h>
#define L 1
#define R 2
#define D 3
#define U 4
const byte RS = 9;
const byte enable = 8;
const byte d4 = 7;
const byte d5 = 6; 
const byte d6 = 13;
const byte d7 = 4;
const byte buzzerPin = 3;
const int dinPin = 12;
const int clockPin = 11;
const int loadPin = 10;
const int pinSW = 2;  // digital pin connected to switch output
const int pinX = A0;  // A0 - analog pin connected to X output
const int pinY = A1;  // A1 - analog pin connected to Y output

const int xPin = A0;
const int yPin = A1;
const int lcdBrightnessPin = 5;

LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);  // DIN, CLK, LOAD, No. DRIVER
LiquidCrystal lcd(RS, enable, d4, d5, d6, d7);
byte matrixBrightness = 2;

byte xPos = 0;
byte yPos = 0;

int debounceDelay = 30;
bool lastChangableState = true;
bool lastStableState = true;
bool commandInUse = false;
unsigned long long lastDebounceTime = 0;
const int resetTimePress = 1000;

const int minThreshold = 200;
const int maxThreshold = 600;

unsigned long long lastMoved = 0;
int lastMeniuOption = -1;
int snakeTailSize = 3;

const byte matrixSize = 8;

bool isFoodEaten =true;

byte matrix[matrixSize][matrixSize] = {
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 }
};

int currentMenuPosition = 0;
int currentPhase = -3;
bool backMenuPressed = false;
 int maxValue = 16;
const int minValue = 0;
int currentSettings[] = { 0, 0, 0, 0 };
int settingAdjustmentVariable = 0;
int lastDisplayedScore=-1;
int currentScore =0;
int currentDirections = 0;
const String welcomeScreen[] = {"Welcome to my", "Snake"};
const String howToUseContinueScreen[] = {"Short click", "for continue"};
const String howToUseBackScreen[]={"Long click","for back"};
const String menuOptionsString[] = { "New Game", "Score Board", "Settings", "About", "How to play" };
const String settingsMenu[] = { "Enter Name", "Difficulty", "LCD brightness", "Matrix brightness", "Sounds Mute" };
String scorBoardMenu[] = { "ABA 0", "AAA 0", "AAA 0", "AAA 0", "AAA 0", "AAA 0", "AAA 0", "AAA 0", "AAA 0", "AAA 0" };
const String aboutText[] = { "Salut eu sunt", "Lefter Ioan", "Cristian si", "asta e Snake-ul", "meu enjoy" };
const String howToPlayText[] = { "Use the Joytick", "To Move and", "try to collect", "the food", "to incress the", "size" };
const int sizeWelcomeScreen =2;
const int sizeHowToUseContinueScreen = 2;
const int sizeHowToUseBackScreen = 2;
const int sizeMenuOption = 5;
const int sizeSettingsMenu = 5;
const int sizeScorBoardMenu = 10;
const int sizeAbout = 5;
const int sizeHowToPlay = 6;
bool inSettingsAdjustment = false;
bool navigationMoved = false;
int snakeTail[64];
unsigned long long lastMovedTime = 0;
int snakeMovementSpeed =700;
int genRow=0;
int genCol=0;
bool userInputReset = false;
bool isGameOver = false;
String currentName = "";
bool winnerScreenDisplay = false;
bool inNameSelection = false;
int cursorPos = 0;
int charIndex = 0;


struct Settings {
  bool soundsMuted;
  char name[4];
  bool difficulty;
  byte lcdBrightness;
  byte matrixBrightness;
} settings;

void setup() {
  Serial.begin(9600);
  // the zero refers to the MAX7219 number, it is zero for 1 chip
  lc.shutdown(0, false);                 // turn off power saving, enables display
  lc.clearDisplay(0);    
    pinMode(pinSW, INPUT_PULLUP);
  pinMode(lcdBrightnessPin , OUTPUT);                
  lcd.begin(16, 2);
  attachInterrupt(digitalPinToInterrupt(pinSW), blink, CHANGE);
  snakeTail[0]=0;
  snakeTail[1]=0;
  snakeTail[2]=0;
  matrix[xPos][yPos] = 1;
  settingLoad();
}
void loop() {
  if (currentPhase != 5) {
    displayCurrentMenu();
  }
  if (currentPhase == 5) {
  gameInputRecord();
  snakeMoves();
  eventGenerator();
  if(lastDisplayedScore!=currentScore){
  displayCurrentScore();
  lastDisplayedScore=currentScore;
  }
  }
  //currentGameStatusCheck();
    // Serial.print("xPos:");
    // Serial.print(xPos);
    // Serial.println("");
    // Serial.print("yPos:");
    // Serial.print(yPos);
    // Serial.println("");
    // Serial.println(isGameOver);
}
void displayCurrentScore(){

  lcd.clear();
    lcd.setCursor(0, 0);
    //lcd.createChar(0, customChar);
    lcd.print("  Current Score:");
    lcd.setCursor(0, 1);
    lcd.print("    ");
    lcd.print(currentScore);
}

void settingLoad(){
  loadDataFromMemory();
   currentSettings[1]=settings.lcdBrightness;
  currentSettings[2]=settings.matrixBrightness;
  currentSettings[0]=settings.difficulty;
  currentSettings[3]=settings.soundsMuted;
    analogWrite(lcdBrightnessPin,map(settings.lcdBrightness,0,16,70,250));
    lc.setIntensity(0, settings.matrixBrightness);  
    currentName = settings.name;
  loadStringsFromEEPROM(scorBoardMenu,10,sizeof(settings));

}

void loadDataFromMemory() {
  EEPROM.get(0, settings);
 // loadStringsFromEEPROM(scorBoardMenu,10,sizeof(settings));
  //Serial.println(scorBoardMenu[0]);
}
 
void saveSettingsinMemory() {
  EEPROM.put(0, settings);
}

int extractInt(String str) {
  int num = 0;
    for (int i = 0; i < str.length(); i++) {
    if (isDigit(str[i])) {
      num = num * 10 + (str[i] - '0');
    }
  }
  return num;
}



void movemantNameSelect(){

    int xValue = analogRead(pinX);
  int yValue = analogRead(pinY);
  if (yValue > maxThreshold  && !userInputReset) {
    charIndex = (charIndex + 1) % 26;
    currentName[cursorPos] = 'A'+charIndex;
    displayName();
    userInputReset = true;
  }
    if (yValue < minThreshold && !userInputReset) {
    charIndex = (charIndex - 1 + 26) % 26;
    currentName[cursorPos] = 'A'+charIndex;
    displayName();
    userInputReset = true;
  }
    if (xValue > maxThreshold && !userInputReset) {
   cursorPos = max(cursorPos - 1, 0);
    displayName();

    userInputReset = true;
  }
    if (xValue < minThreshold  && !userInputReset) {
    cursorPos = min(cursorPos + 1, 2);
    displayName();
    userInputReset = true;
  }
  if (xValue >= minThreshold && xValue <= maxThreshold && yValue >= minThreshold && yValue <= maxThreshold) {
    userInputReset = false;
  }

}
void displayName() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(currentName);
  lcd.setCursor(cursorPos, 1);
  lcd.print("^");
}


void storeStringsInEEPROM(String strings[], int numStrings, int startAddress) {
  for (int i = 0; i < numStrings; i++) {
    int stringLength = strings[i].length();
    // Store the length of the string at the current address
    EEPROM.put(startAddress, stringLength);
    startAddress += sizeof(stringLength);
    // Store the string itself
    for (int j = 0; j < stringLength; j++) {
      EEPROM.put(startAddress + j, strings[i][j]);
    }
    startAddress += stringLength;
  }
}

void loadStringsFromEEPROM(String strings[], int numStrings, int startAddress) {
  for (int i = 0; i < numStrings; i++) {
    int stringLength;
    // Load the length of the string at the current address
    EEPROM.get(startAddress, stringLength);
    startAddress += sizeof(stringLength);
    // Load the string itself
    strings[i] = "";
    for (int j = 0; j < stringLength; j++) {
      char c;
      EEPROM.get(startAddress + j, c);
      strings[i] += c;
    }
    startAddress += stringLength;
  }
}

 

void displayCurrentMenu() {
   if (currentPhase == -3) {
    menuDisplay(welcomeScreen, sizeWelcomeScreen);
  }
   if (currentPhase == -2) {
    menuDisplay(howToUseContinueScreen, sizeHowToUseContinueScreen);
  }
   if (currentPhase == -1) {
    menuDisplay(howToUseBackScreen, sizeHowToUseBackScreen);
  }
  if (currentPhase == 0) {
    menuDisplay(menuOptionsString, sizeMenuOption);
  }
  if (currentPhase == 1) {
    menuDisplay(scorBoardMenu, sizeScorBoardMenu);
  }
  if (currentPhase == 2) {
    if (!inSettingsAdjustment) {
      menuDisplay(settingsMenu, sizeSettingsMenu);
    } else {
      if(!inNameSelection)
      {genericSettings();}
      else{
        movemantNameSelect();
      }
    }
  }
  if (currentPhase == 3) {
    menuDisplay(aboutText, sizeAbout);
  }
  if (currentPhase == 4) {
    menuDisplay(howToPlayText, sizeHowToPlay);
  }
}


void soundEfectClick(){
  if(!settings.soundsMuted){
    tone(buzzerPin, 1000,500);
  }
}

void soundEfectEatFood(){
  if(!settings.soundsMuted){
    tone(buzzerPin, 500,500);
  }
}
void soundEfectGameOver(){
  if(!settings.soundsMuted){
    tone(buzzerPin, 200,500);
  }
}

void menuDisplay(String options[], int size) {
  navigateFunction(size);
  if (lastMeniuOption != currentMenuPosition || backMenuPressed) {
    lcd.clear();
    backMenuPressed = false;
    lcd.setCursor(0, 0);
    //lcd.createChar(0, customChar);
    lcd.print('>' + options[currentMenuPosition]);
    lastMeniuOption = currentMenuPosition;
    if (currentMenuPosition + 1 < size) {
      lcd.setCursor(0, 1);
      lcd.print(' ' + options[currentMenuPosition + 1]);
    }
  }
}

void resetGame(){
  xPos=1;
  yPos=1;
     isFoodEaten=true;
    for(int i = 0 ; i < matrixSize ; ++i){
      for(int j = 0 ; j < matrixSize ; ++j){
      matrix[i][j]=0;}}
    for (int i = 0; i < matrixSize; ++i) {
    for (int j = 0; j < matrixSize; ++j) {
      lc.setLed(0, i, j, 0); }}
    snakeTailSize= 3;
    snakeTail[0]=11;
    snakeTail[1]=1;
    snakeTail[2]=0;
    if(settings.difficulty){
      snakeMovementSpeed=400;
    }
    else
    {snakeMovementSpeed=700;}

    lc.setLed(0, 1, 1, 1);
    lc.setLed(0, 0, 1, 1);
    lc.setLed(0, 0, 0, 1);
    lcd.clear();
     lastDisplayedScore=-1;
     currentScore =0;
    isGameOver=false;
    currentDirections=0;
}

void genericSettings() {
 navigateFunction(maxValue);
  if (lastMeniuOption != currentMenuPosition ) {
    lcd.clear();
    backMenuPressed = false;
    lcd.setCursor(0, 0);
    lcd.print("> ");
  lcd.print(currentMenuPosition);
    lastMeniuOption = currentMenuPosition;
    if (currentMenuPosition - 1 >=0 ) {
      lcd.setCursor(0, 1);
      lcd.print(currentMenuPosition - 1);
    }
  }
}

void updateSettings(){
  settings.lcdBrightness=currentSettings[1];
  settings.matrixBrightness=currentSettings[2];
  settings.difficulty=currentSettings[0];
  currentName.toCharArray(settings.name,4);
  settings.soundsMuted=currentSettings[3];
  saveSettingsinMemory();
  settingLoad();
}

void currentGameStatusCheck(){
  if(xPos<0 || yPos<0 || xPos>matrixSize-1 || yPos>matrixSize-1){
    gameOver();
  }
  if(isGameOver){
    gameOver();
  }
  if(matrix[xPos][yPos]==2){
    snakeTailSize++;
    currentScore++;
    snakeMovementSpeed=max(snakeMovementSpeed-10,200);
    soundEfectEatFood();
    snakeTail[snakeTailSize-1]=snakeTail[snakeTailSize-2];
    isFoodEaten=true;
  }

}

void eventGenerator(){
  if(isFoodEaten){
    if(snakeTailSize<matrixSize*matrixSize/2){
      while(isFoodEaten){
        genRow = random(matrixSize);
        genCol = random(matrixSize);
        if( matrix[genRow][genCol] == 0){
          isFoodEaten=false;
        }
      }
    }
    else{
      int aux[32],count=0;
      for(int i =0 ; i < matrixSize ; ++i)
        {
          for(int j = 0 ; j < matrixSize ; ++j){
            if(matrix[i][j]==0){
              aux[count++]=i*10+j;
            }
          }
        }
      int found = random(count);
      genRow=aux[found]/10%10;
      genCol=aux[found]%10;
    }
    matrix[genRow][genCol]=2;
    lc.setLed(0, genRow, genCol, 1);
    isFoodEaten=false;
  }
}

void gameOver(){
  soundEfectGameOver();
   lcd.clear();
  lcd.setCursor(0,0);
  if(isNewHighSore()){
    lcd.print("HighScore");
    delay(1000);
    addNewHighScore();
  }
  else{
    lcd.print("Game Over!");
    lcd.setCursor(0,1);
    lcd.print("Try Again!");
     delay(1000);
  currentMenuPosition=0;
  currentPhase=1;
  lastMeniuOption=-1;
  }
  matrixCloseLights();
}

bool isNewHighSore(){
  return extractInt(scorBoardMenu[9])<currentScore;
}

void addNewHighScore(){
  String newEntry = currentName+" "+currentScore;
  int j = 0 ;
  for(int i = 0 ; i < 10 ; ++i){
    if(extractInt(scorBoardMenu[i])<currentScore){
        j=i;
        i=11;
    }
  }
  for(int i = 10 ; i> j ; --i){
    scorBoardMenu[i]=scorBoardMenu[i-1];
  }
  scorBoardMenu[j]=newEntry;
  storeStringsInEEPROM(scorBoardMenu,10,sizeof(settings));
  currentMenuPosition=j;
  currentPhase=1;
  lastMeniuOption=-1;
}

void gameMovemantConvertor(){
  if(currentDirections==D){
    xPos++;
  }
  if(currentDirections==U){
    xPos--;
  }
  if(currentDirections==L){
    yPos--;
  }
  if(currentDirections==R){
    yPos++;
  }  
}

void snakeMoves(){
  if(millis()-lastMovedTime>snakeMovementSpeed && currentDirections!=0 ){
    
    gameMovemantConvertor();
    int x=snakeTail[snakeTailSize-1]/10%10;
    int y=snakeTail[snakeTailSize-1]%10;
    matrix[x][y]=0;
    lc.setLed(0, x, y, 0);
    for(int i=snakeTailSize ;  i  > 0 ;  --i){
      snakeTail[i]=snakeTail[i-1];
    }
    currentGameStatusCheck();

    if( matrix[xPos][yPos]==1 && currentDirections!=0){
      isGameOver=true;
    }
    else{
      matrix[xPos][yPos]=1;
 }
    snakeTail[0]=xPos*10+yPos;
    lc.setLed(0, xPos, yPos, 1);
    lastMovedTime=millis();
  }
}

void gameInputRecord() {
  int xValue = analogRead(pinX);
  int yValue = analogRead(pinY);
  if (yValue > maxThreshold &&  currentDirections!=U  && !userInputReset) {
    currentDirections=D;
    userInputReset = true;
  }
    if (yValue < minThreshold  &&  currentDirections!=D && !userInputReset) {
    currentDirections=U;
    userInputReset = true;
  }
    if (xValue > maxThreshold &&  currentDirections!=R && !userInputReset) {
    currentDirections=L;
    userInputReset = true;
  }
    if (xValue < minThreshold &&  currentDirections!=L && !userInputReset) {
    currentDirections=R;
    userInputReset = true;
  }
  if (xValue >= minThreshold && xValue <= maxThreshold && yValue >= minThreshold && yValue <= maxThreshold) {
    userInputReset = false;
  }

}

void navigateFunction(int size) {
  int yValue = analogRead(yPin);

  if (yValue > maxThreshold && !navigationMoved) {
    navigationMoved = !navigationMoved;
    if (currentMenuPosition < size - 1) {
      currentMenuPosition++;
      lcd.clear();
    } else {
      currentMenuPosition = 0;
      lcd.clear();
    }
  } else {
    if (yValue < minThreshold && !navigationMoved) {
      navigationMoved = !navigationMoved;
      if (currentMenuPosition > 0) {
        currentMenuPosition--;
        lcd.clear();

      } else {
        currentMenuPosition = size - 1;
        lcd.clear();
      }
    } else {
      if (navigationMoved && yValue > minThreshold && yValue < maxThreshold) {
        navigationMoved = !navigationMoved;
      }
    }
  }
}

void clickMenu() {
  soundEfectClick();
  if (currentPhase == -3 ) {
    currentPhase = -2;
    currentMenuPosition = 0;
    lastMeniuOption=-1;
  }
  else
  if (currentPhase == -2 ) {
    currentPhase = -1;
    currentMenuPosition = 0;
    lastMeniuOption=-1;
  }
  else
 if (currentPhase == -1 ) {
    currentPhase = 0;
    currentMenuPosition = 0;
    lastMeniuOption=-1;
  }
  else
  if (currentPhase == 0 && currentMenuPosition == 0) {
    currentPhase = 5;
    resetGame();
    currentMenuPosition = 0;
  }
  else
  if (currentPhase == 0 && currentMenuPosition == 1) {
    currentPhase = currentMenuPosition;
    currentMenuPosition = 0;
  }
  else
  if (currentPhase == 0 && currentMenuPosition == 2) {
    currentPhase = currentMenuPosition;
    currentMenuPosition = 0;
  }
  else
  if (currentPhase == 0 && currentMenuPosition == 3) {
    currentPhase = currentMenuPosition;
    currentMenuPosition = 0;
  }
  else
  if (currentPhase == 0 && currentMenuPosition == 4) {
    currentPhase = currentMenuPosition;
    currentMenuPosition = 0;
  }
  else
  if (currentPhase == 2 && currentMenuPosition >= 1 && currentMenuPosition <= 4 && !inSettingsAdjustment) {
    settingAdjustmentVariable = currentMenuPosition - 1;
    if(currentMenuPosition==2 || currentMenuPosition==3){
      maxValue =16;
    }
   if(currentMenuPosition==1 || currentMenuPosition==4){
      maxValue =2;
    }

    currentMenuPosition = currentSettings[currentMenuPosition - 1];
    inSettingsAdjustment = true;
    lcd.clear();
  }
  else
  if (currentPhase == 2 && currentMenuPosition ==0 && !inSettingsAdjustment) {
      inNameSelection=true;
      inSettingsAdjustment = true;
       lcd.clear();
       displayName();
  }
  else
 { if (currentPhase == 2 && inSettingsAdjustment && !inNameSelection) {
    currentSettings[settingAdjustmentVariable] = currentMenuPosition;
    updateSettings();
  }
  else if (currentPhase == 2 && inSettingsAdjustment && inNameSelection) {
    updateSettings();
  }}
  displayCurrentMenu();
}

void backPress() {
  if(currentPhase<=0 && currentPhase >-3){
  currentPhase+=-1;
  currentMenuPosition = 0;
  }
  if (currentPhase == 1 || currentPhase == 2 || currentPhase == 3 || currentPhase == 4) {
    if (inSettingsAdjustment && currentPhase == 2) {
      if(inNameSelection){
        currentName=settings.name;
        inNameSelection=false;
      }
      currentMenuPosition = 0;
      inSettingsAdjustment = false;
      lcd.clear();
    } else {
      currentPhase = 0;
      currentMenuPosition = 0;
    }
  }
  if(currentPhase==5){
    currentPhase=0;
    currentMenuPosition=0;
    lc.clearDisplay(0);    
    matrixCloseLights();
  }
  backMenuPressed = true;
  displayCurrentMenu();
}
void matrixCloseLights(){
  for(int i = 0 ; i< matrixSize ; ++i)
  for(int j = 0 ; j< matrixSize; ++j ){
    lc.setLed(0,i,j,0);
  }
}

void blink() {
  commandInUse = false;
  bool readButtonCurrentState = digitalRead(pinSW);
  if (readButtonCurrentState != lastChangableState) {
    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (lastStableState == LOW && readButtonCurrentState == HIGH) {
        if (millis() - lastDebounceTime > resetTimePress) {
          backPress();
        } else {
          clickMenu();
        }
        commandInUse = true;
      }
      lastStableState = readButtonCurrentState;
    }
    lastDebounceTime = millis();
    lastChangableState = readButtonCurrentState;
  }

  if ((millis() - lastDebounceTime) > debounceDelay && !commandInUse) {
    if (lastStableState == LOW && readButtonCurrentState == HIGH) {
      if (millis() - lastDebounceTime > resetTimePress) {
        backPress();
      } else {
        clickMenu();
      }
    }
    lastStableState = readButtonCurrentState;
  }
}