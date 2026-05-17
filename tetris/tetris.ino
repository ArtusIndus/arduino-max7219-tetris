// Tetris for Arduino Mega + 2x MAX7219 (16x8 LED matrix)
// Delta rendering without flicker

#include <LedControl.h>

#define DATA_PIN 51
#define CLK_PIN  52
#define CS_PIN   53

const int bl = 10;
const int bm = 11;
const int br = 12;

LedControl Anzeige = LedControl(DATA_PIN, CLK_PIN, CS_PIN, 2);

unsigned long lastDropTime = 0;
const int dropInterval = 500;

int currentX, currentY, currentShape;

bool current[3][3];

// Game board
bool board[16][8] = {{0}};

// Screen buffer for delta rendering
bool screen[16][8] = {{0}};

// Tetromino shapes
const bool shapes[7][3][3] = {
  {{1,1,1},{0,0,0},{0,0,0}},
  {{1,1,1},{0,1,0},{0,0,0}},
  {{1,1,0},{0,1,1},{0,0,0}},
  {{1,0,0},{1,1,1},{0,0,0}},
  {{1,1,0},{1,1,0},{0,0,0}},
  {{0,1,1},{1,1,0},{0,0,0}},
  {{0,0,1},{1,1,1},{0,0,0}}
};

// Button states
bool lastBL = HIGH;
bool lastBM = HIGH;
bool lastBR = HIGH;

// Non-blocking debounce timing
unsigned long lastDebounceBL = 0;
unsigned long lastDebounceBM = 0;
unsigned long lastDebounceBR = 0;

const unsigned long debounceDelay = 20;

// Detect button press without delay()
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

// Check collision and borders
bool canMove(int x, int y) {

  for(int i=0;i<3;i++) {

    for(int j=0;j<3;j++) {

      if(current[i][j]) {

        int nx = x + j;
        int ny = y + i;

        if(nx < 0 || nx >= 8 || ny >= 16)
          return false;

        if(ny >= 0 && board[ny][nx])
          return false;
      }
    }
  }

  return true;
}

// Spawn random tetromino
void spawnPiece() {

  static bool firstPiece = true;

  if(firstPiece) {

    randomSeed(
      micros() ^
      analogRead(A0) ^
      analogRead(A1) ^
      analogRead(A2)
    );

    // Throw away first random values
    random(0,1000);
    random(0,1000);
    random(0,1000);

    firstPiece = false;
  }

  currentShape = random(7);

  currentX = 2;
  currentY = 0;

  for(int i=0;i<3;i++) {

    for(int j=0;j<3;j++) {

      current[i][j] = shapes[currentShape][i][j];
    }
  }
}

// Merge current piece into board
void addPieceToBoard() {

  for(int i=0;i<3;i++) {

    for(int j=0;j<3;j++) {

      if(current[i][j]) {

        int x = currentX + j;
        int y = currentY + i;

        if(x >= 0 && x < 8 && y >= 0 && y < 16) {

          board[y][x] = true;
        }
      }
    }
  }
}

// Remove full rows
void clearFullRows() {

  for(int y=15;y>=0;y--) {

    bool full = true;

    for(int x=0;x<8;x++) {

      if(!board[y][x]) {
        full = false;
        break;
      }
    }

    if(full) {

      for(int yy=y;yy>0;yy--) {

        for(int x=0;x<8;x++) {

          board[yy][x] = board[yy-1][x];
        }
      }

      for(int x=0;x<8;x++) {

        board[0][x] = false;
      }

      y++;
    }
  }
}

// Rotate current piece
void rotatePiece() {

  bool temp[3][3];

  for(int i=0;i<3;i++) {

    for(int j=0;j<3;j++) {

      temp[j][2-i] = current[i][j];
    }
  }

  for(int i=0;i<3;i++) {

    for(int j=0;j<3;j++) {

      if(temp[i][j]) {

        int x = currentX + j;
        int y = currentY + i;

        if(x < 0 || x >= 8 || y >= 16)
          return;

        if(y >= 0 && board[y][x])
          return;
      }
    }
  }

  for(int i=0;i<3;i++) {

    for(int j=0;j<3;j++) {

      current[i][j] = temp[i][j];
    }
  }
}

// Draw only changed pixels
void drawBoard() {

  bool newScreen[16][8] = {{0}};

  // Draw board
  for(int y=0;y<16;y++) {

    for(int x=0;x<8;x++) {

      if(board[y][x]) {

        newScreen[y][x] = true;
      }
    }
  }

  // Draw current piece
  for(int i=0;i<3;i++) {

    for(int j=0;j<3;j++) {

      if(current[i][j]) {

        int x = currentX + j;
        int y = currentY + i;

        if(x >= 0 && x < 8 && y >= 0 && y < 16) {

          newScreen[y][x] = true;
        }
      }
    }
  }

  // Update only changed LEDs
  for(int y=0;y<16;y++) {

    for(int x=0;x<8;x++) {

      if(newScreen[y][x] != screen[y][x]) {

        int module = (y < 8) ? 1 : 0;
        int row = (y < 8) ? y : y - 8;

        Anzeige.setLed(module, row, x, newScreen[y][x]);

        screen[y][x] = newScreen[y][x];
      }
    }
  }
}

void setup() {

  // Initialize MAX7219 modules
  for(int m=0;m<2;m++) {

    Anzeige.shutdown(m, false);
    Anzeige.setIntensity(m, 4);
    Anzeige.clearDisplay(m);
  }

  // Initialize buttons
  pinMode(bl, INPUT_PULLUP);
  pinMode(bm, INPUT_PULLUP);
  pinMode(br, INPUT_PULLUP);

  // Random seed from floating analog pins
  long seed = 0;

for(int i = 0; i < 32; i++) {
  seed ^= analogRead(A0) << (i % 8);
  delay(1);
}

randomSeed(seed ^ micros());

  Serial.begin(9600);
  Serial.print(    analogRead(A0));

  spawnPiece();

  lastDropTime = millis();
}

void loop() {

  unsigned long now = millis();

  // Move left
  if(pressed(bl, lastBL, lastDebounceBL)) {

    if(canMove(currentX - 1, currentY)) {

      currentX--;
    }
  }

  // Move right
  if(pressed(br, lastBR, lastDebounceBR)) {

    if(canMove(currentX + 1, currentY)) {

      currentX++;
    }
  }

  // Rotate
  if(pressed(bm, lastBM, lastDebounceBM)) {

    rotatePiece();
  }

  // Automatic falling
  if(now - lastDropTime >= dropInterval) {

    if(canMove(currentX, currentY + 1)) {

      currentY++;

    } else {

      addPieceToBoard();

      clearFullRows();

      spawnPiece();

      // Game over reset
      if(!canMove(currentX, currentY)) {

        for(int y=0;y<16;y++) {

          for(int x=0;x<8;x++) {

            board[y][x] = false;
          }
        }
      }
    }

    lastDropTime = now;
  }

  drawBoard();
}