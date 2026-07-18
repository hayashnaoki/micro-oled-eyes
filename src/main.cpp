#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>

// Explicitly define pins for readable code
#define PIN_OLED_RST 29 // Shared hardware reset line
#define PIN_OLED_DC 6   // Shared data/command line
#define PIN_LEFT_CS 27  // Dedicated left eye select
#define PIN_RIGHT_CS 28 // Dedicated right eye select

// Note: Set the reset pin argument to U8X8_PIN_NONE so U8g2 doesn't wipe out
// the other display
U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI leftEye(U8G2_R0, /* cs= */ PIN_LEFT_CS,
                                              /* dc= */ PIN_OLED_DC,
                                              /* reset= */ U8X8_PIN_NONE);
U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI rightEye(U8G2_R0, /* cs= */ PIN_RIGHT_CS,
                                               /* dc= */ PIN_OLED_DC,
                                               /* reset= */ U8X8_PIN_NONE);

const int centerX = 64;
const int centerY = 32;

void setup() {
  // 1. Manually reset BOTH displays simultaneously using the shared line
  pinMode(PIN_OLED_RST, OUTPUT);
  digitalWrite(PIN_OLED_RST, LOW);  // Pull low to reset
  delay(50);                        // Hold reset state
  digitalWrite(PIN_OLED_RST, HIGH); // Pull high to wake up screens
  delay(50);                        // Wait for controller stable state

  // 2. Initialize the software buffers safely now that both displays are awake
  leftEye.begin();
  rightEye.begin();
}

void drawEye(U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI &display, int pupilX,
             int pupilY) {
  display.clearBuffer();

  display.drawDisc(centerX, centerY, 30, U8G2_DRAW_ALL);
  display.setDrawColor(0);
  display.drawDisc(centerX, centerY, 28, U8G2_DRAW_ALL);

  display.setDrawColor(1);
  display.drawDisc(pupilX, pupilY, 12, U8G2_DRAW_ALL);
  display.setDrawColor(0);
  display.drawDisc(pupilX - 4, pupilY - 4, 3, U8G2_DRAW_ALL);

  display.sendBuffer();
}

void loop() {
  int offsetX = sin(millis() * 0.0015) * 14;
  int offsetY = cos(millis() * 0.001) * 8;

  int targetX = centerX + offsetX;
  int targetY = centerY + offsetY;

  drawEye(leftEye, targetX, targetY);
  drawEye(rightEye, targetX, targetY);

  delay(16);
}