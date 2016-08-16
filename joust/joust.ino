#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Used for software SPI for accelerometer, Adafruit LIS3DH
#define LIS3DH_CLK 2
#define LIS3DH_MISO 0
#define LIS3DH_MOSI 1
#define LIS3DH_CS 3
#define LED_PIN 4

Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_LIS3DH lis = Adafruit_LIS3DH(LIS3DH_CS, LIS3DH_MOSI, LIS3DH_MISO, LIS3DH_CLK);

const float gravity = 9.3; // m/s^2, not quite
const uint32_t white = strip.Color(255,255,255);
const uint32_t black = strip.Color(0,0,0); 
const uint32_t red = strip.Color(255,0,0);
const uint32_t blue = strip.Color(0,0,255);
const uint32_t green = strip.Color(0,255,0);

// Game play constants.
const int max_lives = 30;
const int penalty_rate = 1;
const int recovery_rate = 2 * penalty_rate;
const double threshold = 1.0; // lower value means easier to lose

// Global variables.
int lives = max_lives;

void setup(void) {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  
  if (! lis.begin(0x18)) {   // change this to 0x19 for alternative i2c address
    blinkTwoColors(red, black, 300, 5);
    while (1);
  }
  lis.setRange(LIS3DH_RANGE_4_G);   // 2, 4, 8 or 16 G!

  // Count down
  blinkTwoColors(green, black, 300, 5);
}

void loop() {
  displayLives(lives);
  double magnitude = readMagnitude();
  int lifeDiff = magnitudeLifeDiff(magnitude);
  lives = constrain(lives+lifeDiff, 0, max_lives);
  if (lives == 0) {
    blinkTwoColors(red, white, 300, 5);
    setColor(red);
    while(1);
  }
}

int magnitudeLifeDiff (double mag) {
  if (mag > threshold) {
    return -1 * penalty_rate;
  } else {
    return recovery_rate;
  }
}

void blinkTwoColors (uint32_t first, uint32_t second, int duration, int times) {
  for (int i = 0; i < times; i++) {
    setColor(first);
    delay(duration);
    setColor(second);
    delay(duration);
  }
}

void setColor (uint32_t color) {
  strip.setPixelColor(0, color);
  strip.show();
}

void displayLives(int n) {
  setColor(strip.Color(255 * (max_lives-n) / max_lives, 0, 255 * n / max_lives));
}

double readMagnitude () {
  lis.read();
  sensors_event_t event; 
  lis.getEvent(&event);
  return abs(sqrt(pow(event.acceleration.x,2)
                 + pow(event.acceleration.y,2)
                 + pow(event.acceleration.z,2)) - gravity);
}

