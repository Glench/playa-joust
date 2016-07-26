// Accelerometer libraries
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

// Used for software SPI for accelerometer, Adafruit LIS3DH
#define LIS3DH_CLK 2
#define LIS3DH_MISO 1
#define LIS3DH_MOSI 0
// Used for hardware & software SPI
#define LIS3DH_CS 3

// software SPI
Adafruit_LIS3DH lis = Adafruit_LIS3DH(LIS3DH_CS, LIS3DH_MOSI, LIS3DH_MISO, LIS3DH_CLK);
// hardware SPI
//Adafruit_LIS3DH lis = Adafruit_LIS3DH(LIS3DH_CS);
// I2C
//Adafruit_LIS3DH lis = Adafruit_LIS3DH();

#if defined(ARDUINO_ARCH_SAMD)
// for Zero, output on USB Serial console, remove line below if using programming port to program the Zero!
   #define Serial SerialUSB
#endif

// Neopixel libraries
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define LED_PIN 4

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup(void) {
  // accelerometer setup code
/*#ifndef ESP8266
  while (!Serial);     // will pause Zero, Leonardo, etc until serial console opens
#endif

  Serial.begin(9600);
  Serial.println("LIS3DH test!");
*/  
  if (! lis.begin(0x18)) {   // change this to 0x19 for alternative i2c address
    // Serial.println("Couldnt start");
    while (1);
  }
  //Serial.println("LIS3DH found!");
  
  lis.setRange(LIS3DH_RANGE_4_G);   // 2, 4, 8 or 16 G!
  
  //Serial.print("Range = "); Serial.print(2 << lis.getRange());  
  //Serial.println("G");

  // neopixel setup code
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}


float gravity = 9.3; // m/s^2, not quite 
double magnitude = 0;
double threshhold = 25; // lower value means easier to lose
bool hasLost = false;
uint32_t color;
uint32_t blinkColor = strip.Color(255,255,255);
unsigned long timeWhenLost;
unsigned long lastBlinkTime = 0;
bool hasBlinked = false;

void loop() {
  // read accelerometer data
  lis.read();      // get X Y and Z data at once
  // Then print out the raw data
  //Serial.print("X:  "); Serial.print(lis.x); 
  //Serial.print("  \tY:  "); Serial.print(lis.y); 
  //Serial.print("  \tZ:  "); Serial.print(lis.z);

  /* Or....get a new sensor event, normalized */ 
  sensors_event_t event; 
  lis.getEvent(&event);

  // get the magnitude of the vectors minus gravity, and use the absolute value of that. or decay magnitude
  magnitude = max(abs(sqrt(pow(event.acceleration.x,2) + pow(event.acceleration.y,2) + pow(event.acceleration.z,2)) - gravity), magnitude*0.980);
    
  // start off blue and smoothly transition to red the harder the controller's been shaken
  color = strip.Color(map(round(magnitude),0,20,0,255),0,255-map(round(magnitude),0,20,0,255));

  if (magnitude > threshhold && !hasLost) {
    hasLost = true;
    timeWhenLost = millis();
  }

  if (hasLost) {
    if (millis() - timeWhenLost < 3000 && !hasBlinked) { // the numerical constant changes length of blink mode, should be about 1 second
      if (millis() - lastBlinkTime > 100) { // the numerical constant changes duration of a color in blink, should be about 100ms
        // flip back and forth between colors for a bit to indicate lost state
        blinkColor = blinkColor == strip.Color(255,0,0) ? strip.Color(255,255,255) : strip.Color(255,0,0);
        lastBlinkTime = millis();  
      }
      color = blinkColor;
      
      // TODO: start vibrator motor
      
    } else {
      // steady color in lost state
      hasBlinked = true;
      color = strip.Color(255,0,0);

      // TODO: stop vibrator motor
    }
  }

  strip.setPixelColor(0, color);
  strip.show();  
}
