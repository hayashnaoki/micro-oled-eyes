#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>

#define PIN_OLED_RST 29
#define PIN_OLED_DC 6
#define PIN_LEFT_CS 27
#define PIN_RIGHT_CS 28

U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI leftEye(U8G2_R0, PIN_LEFT_CS, PIN_OLED_DC,
                                              U8X8_PIN_NONE);
U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI rightEye(U8G2_R0, PIN_RIGHT_CS,
                                               PIN_OLED_DC, U8X8_PIN_NONE);

const int centerX = 64;
const int centerY = 32;
const int inwardShift = 6; // Shift pupil inward by 6 pixels for natural convergence
const int maxGazeOffset = 18; // Maximum gaze travel from the center
const int pupilSpeed = 4; // Speed of pupil horizontal movement (pixels per frame)
const float bounceSpeed = 0.003f; // Speed of the idle vertical bounce
const float bounceAmplitude = 3.0f; // Amplitude of the idle vertical bounce (in pixels)

// --- Timing Variables (Non-blocking) ---
unsigned long lastBlinkTime = 0;
unsigned long lastSearchTime = 0;
unsigned long searchDurationStart = 0;

const unsigned long blinkInterval = 4000;   // Blink every 4 seconds
const unsigned long blinkDuration = 200;    // Eye stays shut for 200ms
unsigned long nextSearchInterval = 10000;   // Dynamic interval between search sequences
const unsigned long searchDuration = 1000;  // Duration for each direction look

// --- State Variables ---
bool isBlinking = false;
int searchStep = 0;                         // 0 = idle, 1 = first look direction, 2 = second look direction
bool searchFirstLookLeft = true;            // Direction of the first look in a sequence
int currentPupilX = centerX;
int currentPupilY = centerY;
int targetPupilX = centerX;

void setup() {
  pinMode(PIN_OLED_RST, OUTPUT);
  digitalWrite(PIN_OLED_RST, LOW);
  delay(50);
  digitalWrite(PIN_OLED_RST, HIGH);
  delay(50);

  leftEye.begin();
  rightEye.begin();

  // Seed the random generator using noise from an unconnected analog pin
  randomSeed(analogRead(26));
  nextSearchInterval = random(5000, 15000); // Randomize first search start time
}

// Custom rendering drawing a Rounded Square Eye
void drawRoundedEye(U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI &display, int pX,
                    int pY, float blinkScale) {
  display.clearBuffer();

  // 1. Calculate height and Y position based on blinkScale
  int baseHeight = 54;
  int h = baseHeight * blinkScale;
  if (h < 4)
    h = 4; // Keep a thin line when fully closed

  int y = centerY - h / 2;

  // Calculate corner radius (normally 12)
  int r = 12 * blinkScale;
  if (r > h / 2)
    r = h / 2; // Ensure corner radius isn't larger than half the height

  // 2. Draw Outer White Rounded Box
  display.setDrawColor(1);
  display.drawRBox(32, y, 64, h, r);

  // 3. Draw Solid Black Circular Pupil (Radius: 10) inside the boundary
  // Only draw if there's enough height to show it cleanly
  if (h > 20) {
    display.setDrawColor(0);
    display.drawDisc(pX, pY, 10, U8G2_DRAW_ALL);
  }

  display.sendBuffer();
}

void loop() {
  unsigned long currentTime = millis();
  float blinkScale = 1.0f;

  // --- 1. BLINK LOGIC SEQUENCE WITH SINE EASING ---
  if (!isBlinking && (currentTime - lastBlinkTime >= blinkInterval)) {
    isBlinking = true;
    lastBlinkTime = currentTime;
  }

  if (isBlinking) {
    unsigned long elapsed = currentTime - lastBlinkTime;
    if (elapsed >= blinkDuration) {
      isBlinking = false;
      blinkScale = 1.0f;
    } else {
      float t = (float)elapsed / (float)blinkDuration;
      // Smoothly close and open using cosine-based easing:
      // Starts at 1.0, smoothly drops to 0.0 at mid-blink, then smoothly
      // returns to 1.0
      blinkScale = 0.5f + 0.5f * cosf(t * 2.0f * 3.14159265f);
    }
  }

  // --- 2. SWIFT SEARCH LOGIC SEQUENCE WITH STATE MACHINE ---
  if (searchStep == 0 && (currentTime - lastSearchTime >= nextSearchInterval)) {
    // Start search sequence (First look)
    searchStep = 1;
    searchDurationStart = currentTime;

    // Randomly decide first look direction (true = left, false = right)
    searchFirstLookLeft = (random(0, 2) == 0);

    if (searchFirstLookLeft) {
      targetPupilX = centerX - maxGazeOffset;
    } else {
      targetPupilX = centerX + maxGazeOffset;
    }
  }

  if (searchStep == 1 && (currentTime - searchDurationStart >= searchDuration)) {
    // Transition to second look direction (opposite direction)
    searchStep = 2;
    searchDurationStart = currentTime;

    if (searchFirstLookLeft) {
      targetPupilX = centerX + maxGazeOffset;
    } else {
      targetPupilX = centerX - maxGazeOffset;
    }
  }

  // If searching duration expires on the second look, return focus back to the center
  if (searchStep == 2 && (currentTime - searchDurationStart >= searchDuration)) {
    searchStep = 0;
    lastSearchTime = currentTime;
    
    // Randomize the next search interval (between 6 and 15 seconds)
    nextSearchInterval = random(6000, 15000);
    
    targetPupilX = centerX;
  }

  // --- 3. SMOOTH MOVEMENT INTERPOLATION (Easing) ---
  // Slowly step current position toward target position to make look movements
  // rapid but organic
  if (currentPupilX < targetPupilX) {
    currentPupilX += pupilSpeed;
    if (currentPupilX > targetPupilX) currentPupilX = targetPupilX;
  }
  if (currentPupilX > targetPupilX) {
    currentPupilX -= pupilSpeed;
    if (currentPupilX < targetPupilX) currentPupilX = targetPupilX;
  }

  // --- 4. CONSTANT VERTICAL BOUNCE ---
  currentPupilY = centerY + (int)(sinf(currentTime * bounceSpeed) * bounceAmplitude);

  // --- 5. RENDER FRAME TO BOTH SCREENS ---
  // Scale down the inwardShift as the pupil moves towards the gaze extremes
  // This keeps the travel range symmetric and avoids hitting the borders
  // unevenly.
  float shiftFactor =
      1.0f - ((float)abs(currentPupilX - centerX) / (float)maxGazeOffset);
  if (shiftFactor < 0.0f)
    shiftFactor = 0.0f;
  int appliedShift = (int)(inwardShift * shiftFactor);

  drawRoundedEye(leftEye, currentPupilX + appliedShift, currentPupilY,
                 blinkScale);
  drawRoundedEye(rightEye, currentPupilX - appliedShift, currentPupilY,
                 blinkScale);

  delay(16); // Maintain ~60 frames per second loop speed
}