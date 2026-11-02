#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width in pixels
#define SCREEN_HEIGHT 64 // OLED display height in pixels


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


// Jump Button Pin Definition
#define JUMP_BUTTON_PIN 16 

// --- Game State Variables ---
int stickFigureX = 20;
int stickFigureY = 0;
int groundLevel = SCREEN_HEIGHT - 10;

float velocityY = 0;
bool isJumping = false;

int runFrame = 0;
unsigned long lastAnimationUpdateTime = 0;
const unsigned long ANIMATION_FRAME_DELAY = 100;

int obstacleX = SCREEN_WIDTH;
int obstacleWidth = 10;
int obstacleHeight = 15;
const int OBSTACLE_SPEED = 2;

long score = 0;
bool gameOver = false;

// --- Function Prototypes ---
void drawStickFigure(int x, int y, int frame, bool jumping);
void drawGround();
void drawObstacle();
void drawScore();
void handleJumpLogic();
void handleObstacleLogic();
void resetGame();

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Stick Figure Runner...");

  pinMode(JUMP_BUTTON_PIN, INPUT_PULLUP); // Set button pin as input with internal pull-up resistor

  

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  Serial.println("OLED display initialized.");

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Loading...");
  display.display();
  delay(1000);

  resetGame();
  Serial.println("Game reset.");
}

void loop() {
  if (!gameOver) {
    // --- Update Game State ---
    handleJumpLogic();
    handleObstacleLogic();

    // Update running animation frame
    if (millis() - lastAnimationUpdateTime > ANIMATION_FRAME_DELAY) {
      if (!isJumping) {
        runFrame = 1 - runFrame;
      }
      lastAnimationUpdateTime = millis();
    }

    // --- Drawing ---
    display.clearDisplay();

    drawGround();
    drawObstacle();
    drawStickFigure(stickFigureX, groundLevel - stickFigureY, runFrame, isJumping);
    drawScore();

    display.display();

  } else {
    // Game Over screen
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(20, 15);
    display.println("GAME OVER");
    display.setTextSize(1);
    display.setCursor(30, 40);
    display.print("Score: ");
    display.println(score);
    display.display();

    // --- NEW: Check for button press to restart ---
    if (digitalRead(JUMP_BUTTON_PIN) == LOW) { // If jump button is pressed
      resetGame(); // Reset all game variables
      delay(200); // Small delay to prevent immediate re-trigger
      Serial.println("Game Restarted by button press!"); 
    }
  }
}

// --- Drawing Functions (No changes) ---
void drawStickFigure(int x, int y, int frame, bool jumping) {
  display.drawCircle(x, y - 20, 4, WHITE);
  display.drawLine(x, y - 16, x, y - 5, WHITE);

  if (jumping) {
    display.drawLine(x, y - 12, x - 5, y - 18, WHITE);
    display.drawLine(x, y - 12, x + 5, y - 18, WHITE);
    display.drawLine(x, y - 5, x - 3, y + 2, WHITE);
    display.drawLine(x, y - 5, x + 3, y + 2, WHITE);
  } else {
    if (frame == 0) {
      display.drawLine(x, y - 12, x - 6, y - 8, WHITE);
      display.drawLine(x, y - 12, x + 6, y - 16, WHITE);
    } else {
      display.drawLine(x, y - 12, x - 6, y - 16, WHITE);
      display.drawLine(x, y - 12, x + 6, y - 8, WHITE);
    }
    if (frame == 0) {
      display.drawLine(x, y - 5, x - 4, y + 2, WHITE);
      display.drawLine(x, y - 5, x + 4, y + 5, WHITE);
    } else {
      display.drawLine(x, y - 5, x - 4, y + 5, WHITE);
      display.drawLine(x, y - 5, x + 4, y + 2, WHITE);
    }
  }
}

void drawGround() {
  display.drawLine(0, groundLevel, SCREEN_WIDTH - 1, groundLevel, WHITE);
}

void drawObstacle() {
  display.fillRect(obstacleX, groundLevel - obstacleHeight, obstacleWidth, obstacleHeight, WHITE);
}

void drawScore() {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(SCREEN_WIDTH - 50, 5);
  display.print("SCORE: ");
  display.println(score);
}

// --- Game Logic Functions ---
void handleJumpLogic() {
  const float JUMP_VELOCITY = 4.5;
  const float GRAVITY = 0.2;

  if (isJumping) {
    stickFigureY += velocityY;
    velocityY -= GRAVITY;

    if (stickFigureY <= 0) {
      stickFigureY = 0;
      velocityY = 0;
      isJumping = false;
    }
  } else {
    // Check for Jump Button Press
    if (digitalRead(JUMP_BUTTON_PIN) == LOW) {
      if (stickFigureY == 0) { // Only allow jump if on the ground
        isJumping = true;
        velocityY = JUMP_VELOCITY;
        // Serial.println("Jump Button Pressed!"); 
      }
    }
  }
}

void handleObstacleLogic() {
  obstacleX -= OBSTACLE_SPEED;

  if (obstacleX + obstacleWidth < 0) {
    obstacleX = SCREEN_WIDTH + random(50, 150);
    obstacleHeight = random(10, 25);
    score++;
  }

  bool horizontalOverlap = (stickFigureX + 5 > obstacleX) && (stickFigureX - 5 < obstacleX + obstacleWidth);
  bool verticalOverlap = (stickFigureY < obstacleHeight);

  if (horizontalOverlap && verticalOverlap) {
    if (!gameOver) {
      Serial.println("COLLISION DETECTED! Game Over.");
    }
    gameOver = true;
  }
}

void resetGame() {
  stickFigureX = 20;
  stickFigureY = 0;
  velocityY = 0;
  isJumping = false;
  runFrame = 0;
  obstacleX = SCREEN_WIDTH;
  obstacleWidth = 10;
  obstacleHeight = 15;
  score = 0;
  gameOver = false;
  lastAnimationUpdateTime = millis();
  randomSeed(analogRead(0));
}

