#include <LedControl.h>
#include <Arduino.h>
#include <TM1637Display.h>

#define BUZZER_PIN 6
#define DATA_PIN 51
#define CLK_PIN  52
#define CS_PIN   53

#define CLK2 48
#define DIO2 49

TM1637Display display(CLK2, DIO2);

// Buttons
const int bl = 10;
const int bm = 11;
const int br = 12;

LedControl Anzeige = LedControl(DATA_PIN, CLK_PIN, CS_PIN, 2);

// ---------------- GAME STATE ----------------
unsigned long lastDropTime = 0;
const int dropInterval = 500;

int currentX, currentY, currentShape;
int score = 0;

bool current[3][3];

bool board[16][8] = {{0}};
bool screen[16][8] = {{0}};

const bool shapes[7][3][3] = {
  {{1,1,1},{0,0,0},{0,0,0}},
  {{1,1,1},{0,1,0},{0,0,0}},
  {{1,1,0},{0,1,1},{0,0,0}},
  {{1,0,0},{1,1,1},{0,0,0}},
  {{1,1,0},{1,1,0},{0,0,0}},
  {{0,1,1},{1,1,0},{0,0,0}},
  {{0,0,1},{1,1,1},{0,0,0}}
};

// ---------------- INPUT ----------------
bool lastBL = HIGH, lastBM = HIGH, lastBR = HIGH;
unsigned long lastDebounceBL = 0, lastDebounceBM = 0, lastDebounceBR = 0;
const unsigned long debounceDelay = 20;

// ---------------- CLEAR ----------------
bool clearingRows = false;
bool rowsToClear[16] = {false};
int clearFlash = 0;
bool clearVisible = false;
unsigned long clearTimer = 0;
const unsigned long clearInterval = 80;

// ---------------- GAME OVER ----------------
bool gameOver = false;
int gameOverFlash = 0;
unsigned long gameOverTimer = 0;

// ---------------- SOUND ----------------
unsigned long sound_timer = 0;

void beep(int freq, int duration) {

  if(millis() - sound_timer >= 200) {

    sound_timer = millis();

    tone(BUZZER_PIN, freq, duration);
  }
}

// ---------------- INPUT ----------------
bool pressed(int pin, bool &lastState, unsigned long &lastDebounce) {
  bool now = digitalRead(pin);

  if(lastState == HIGH && now == LOW) {
    if(millis() - lastDebounce > debounceDelay) {
      lastDebounce = millis();
      lastState = now;
      return true;
    }
  }

  lastState = now;
  return false;
}

// ---------------- COLLISION ----------------
bool canMove(int x, int y) {
  for(int i = 0; i < 3; i++) {
    for(int j = 0; j < 3; j++) {
      if(current[i][j]) {
        int nx = x + j;
        int ny = y + i;

        if(nx < 0 || nx >= 8 || ny >= 16) return false;
        if(ny >= 0 && board[ny][nx]) return false;
      }
    }
  }
  return true;
}

// ---------------- SPAWN ----------------
void spawnPiece() {
  currentShape = random(7);
  currentX = 2;
  currentY = 0;

  for(int i = 0; i < 3; i++)
    for(int j = 0; j < 3; j++)
      current[i][j] = shapes[currentShape][i][j];
}

// ---------------- BOARD MERGE ----------------
void addPieceToBoard() {
  for(int i = 0; i < 3; i++) {
    for(int j = 0; j < 3; j++) {
      if(current[i][j]) {
        int x = currentX + j;
        int y = currentY + i;

        if(x >= 0 && x < 8 && y >= 0 && y < 16)
          board[y][x] = true;
      }
    }
  }
}

// ---------------- CLEAR ROWS ----------------
void clearFullRows() {
  if(clearingRows) return;

  bool found = false;

  for(int y = 0; y < 16; y++) {
    bool full = true;

    for(int x = 0; x < 8; x++) {
      if(!board[y][x]) {
        full = false;
        break;
      }
    }

    rowsToClear[y] = full;
    if(full) found = true;
  }

  if(found) {
    clearingRows = true;
    clearFlash = 0;
    clearVisible = false;
    clearTimer = millis();
  }
}

// ---------------- GAME OVER ANIMATION ----------------
void triggerGameOver() {
  gameOver = true;
  gameOverFlash = 0;
  gameOverTimer = millis();
}

void updateGameOver() {
  if(!gameOver) return;

  if(millis() - gameOverTimer < 200) return;
  gameOverTimer = millis();

  gameOverFlash++;

  if(gameOverFlash % 2 == 0) {
    Anzeige.clearDisplay(0);
    Anzeige.clearDisplay(1);
  } else {
    // show all ON
    for(int y = 0; y < 8; y++) {
      for(int x = 0; x < 8; x++) {
        Anzeige.setLed(0, y, x, 1);
        Anzeige.setLed(1, y, x, 1);
      }
    }
  }

  if(gameOverFlash > 8) {

    // Animation beenden
    gameOver = false;

    // Hardware displays clearen
    Anzeige.clearDisplay(0);
    Anzeige.clearDisplay(1);

    // Board löschen
    for(int y = 0; y < 16; y++) {

      for(int x = 0; x < 8; x++) {

        board[y][x] = false;
      }
    }

    // Neue Figur sofort erzeugen
    spawnPiece();

    // Screen Buffer absichtlich falsch setzen
    // damit drawBoard alles komplett neu zeichnet
    for(int y = 0; y < 16; y++) {

      for(int x = 0; x < 8; x++) {

        screen[y][x] = !board[y][x];
      }
    }

    // Kompletten Frame neu rendern
    drawBoard();

    // Score reset
    score = 0;
    display.showNumberDec(score, false);

    // Timer reset
    lastDropTime = millis();
  }
}

// ---------------- CLEAR ANIMATION ----------------
void updateRowClearAnimation() {
  if(!clearingRows) return;
  if(millis() - clearTimer < clearInterval) return;

  clearTimer = millis();
  clearVisible = !clearVisible;
  clearFlash++;

  if(clearFlash >= 6) {

    int cleared = 0;
    for(int y = 0; y < 16; y++)
      if(rowsToClear[y]) cleared++;

    score += cleared;
    display.showNumberDec(score, false);

    if(cleared == 1) {
      tone(BUZZER_PIN, 900, 80);
    } else if(cleared == 2) {
      tone(BUZZER_PIN, 1100, 120);
    } else if(cleared >= 3) {
      tone(BUZZER_PIN, 1400, 180);
    }

    bool temp[16][8] = {{0}};
    int writeY = 15;

    for(int y = 15; y >= 0; y--) {
      if(!rowsToClear[y]) {
        for(int x = 0; x < 8; x++)
          temp[writeY][x] = board[y][x];
        writeY--;
      }
    }

    for(int y = 0; y < 16; y++)
      for(int x = 0; x < 8; x++)
        board[y][x] = temp[y][x];

    for(int y = 0; y < 16; y++)
      rowsToClear[y] = false;

    for(int y = 0; y < 16; y++)
      for(int x = 0; x < 8; x++)
        screen[y][x] = false;

    Anzeige.clearDisplay(0);
    Anzeige.clearDisplay(1);

    clearingRows = false;
    spawnPiece();

    if(!canMove(currentX, currentY)) {
      triggerGameOver();
    }
  }
}

// ---------------- ROTATE ----------------
void rotatePiece() {
  bool temp[3][3];

  for(int i = 0; i < 3; i++)
    for(int j = 0; j < 3; j++)
      temp[j][2 - i] = current[i][j];

  for(int i = 0; i < 3; i++) {
    for(int j = 0; j < 3; j++) {
      if(temp[i][j]) {
        int x = currentX + j;
        int y = currentY + i;

        if(x < 0 || x >= 8 || y >= 16) return;
        if(y >= 0 && board[y][x]) return;
      }
    }
  }

  for(int i = 0; i < 3; i++)
    for(int j = 0; j < 3; j++)
      current[i][j] = temp[i][j];
}

// ---------------- DRAW ----------------
void drawBoard() {
  bool newScreen[16][8] = {{0}};

  for(int y = 0; y < 16; y++)
    for(int x = 0; x < 8; x++)
      if(board[y][x]) newScreen[y][x] = true;

  if(!clearingRows && !gameOver) {
    for(int i = 0; i < 3; i++) {
      for(int j = 0; j < 3; j++) {
        if(current[i][j]) {
          int x = currentX + j;
          int y = currentY + i;
          if(x >= 0 && x < 8 && y >= 0 && y < 16)
            newScreen[y][x] = true;
        }
      }
    }
  }

  if(clearingRows && !clearVisible) {
    for(int y = 0; y < 16; y++)
      if(rowsToClear[y])
        for(int x = 0; x < 8; x++)
          newScreen[y][x] = false;
  }

  for(int y = 0; y < 16; y++) {
    for(int x = 0; x < 8; x++) {
      if(newScreen[y][x] != screen[y][x]) {
        int module = (y < 8) ? 1 : 0;
        int row = (y < 8) ? y : y - 8;

        Anzeige.setLed(module, row, x, newScreen[y][x]);
        screen[y][x] = newScreen[y][x];
      }
    }
  }
}

// ---------------- SETUP ----------------
void setup() {
  for(int m = 0; m < 2; m++) {
    Anzeige.shutdown(m, false);
    Anzeige.setIntensity(m, 4);
    Anzeige.clearDisplay(m);
  }

  display.setBrightness(7);
  display.showNumberDec(0, false);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(bl, INPUT_PULLUP);
  pinMode(bm, INPUT_PULLUP);
  pinMode(br, INPUT_PULLUP);

  randomSeed(micros());
  spawnPiece();
  lastDropTime = millis();
}

// ---------------- LOOP ----------------
void loop() {
  unsigned long now = millis();

  updateGameOver();
  updateRowClearAnimation();

  if(!clearingRows && !gameOver) {

    // LEFT
if(pressed(bl, lastBL, lastDebounceBL)) {

  if(canMove(currentX - 1, currentY)) {

    currentX--;

    beep(800, 20);
  }
}

// RIGHT
if(pressed(br, lastBR, lastDebounceBR)) {

  if(canMove(currentX + 1, currentY)) {

    currentX++;

    beep(800, 20);
  }
}

// ROTATE
if(pressed(bm, lastBM, lastDebounceBM)) {

  rotatePiece();

  beep(1200, 30);
}

    if(now - lastDropTime >= dropInterval) {

      if(canMove(currentX, currentY + 1)) {
        currentY++;
      } else {
        addPieceToBoard();
        clearFullRows();

        if(!clearingRows) {
          spawnPiece();

          if(!canMove(currentX, currentY)) {
            triggerGameOver(); beep(150, 500);
          }
        }
      }

      lastDropTime = now;
    }
  }

  drawBoard();
}