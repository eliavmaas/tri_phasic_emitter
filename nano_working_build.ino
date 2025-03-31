#include <FastLED.h>
#include <arduinoFFT.h>

#define NUM_LEDS 10              // Number of LEDs
#define DATA_PIN 6               // Data pin for the LED strip
#define AUDIO_PIN A0             // Audio input pin
#define POT_PIN A1               // Potentiometer pin for threshold control
#define COLOR_POT_PIN A2         // Second potentiometer for color sensitivity control
#define BUTTON_PIN 2             // Button pin to switch modes
#define SAMPLING_FREQUENCY 5000  // Sampling frequency for FFT
#define SAMPLES 32               // Number of samples for FFT

CRGB leds[NUM_LEDS];         // Array to store LED colors
double vReal[SAMPLES];       // Array for real FFT values
double vImag[SAMPLES];       // Array for imaginary FFT values
int baseBrightness = 75;     // Base brightness of the LEDs
int threshold = -350;        // Sound threshold to trigger LED effects
int mode = 0;                // Mode selection variable
int colorSensitivity = 255;  // Color sensitivity default value

ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, SAMPLES, SAMPLING_FREQUENCY);

unsigned long lastPrintTime = 0;

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(baseBrightness);
  pinMode(AUDIO_PIN, INPUT);
  pinMode(POT_PIN, INPUT);
  pinMode(COLOR_POT_PIN, INPUT);      // New color sensitivity potentiometer
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Internal pull-up resistor for the button
  Serial.println("Setup complete");
}

void loop() {
  int potValue = analogRead(POT_PIN);
  threshold = map(potValue, 0, 1023, -1000, -100);  // Map potentiometer value to threshold

  int colorPotValue = analogRead(COLOR_POT_PIN);
  colorSensitivity = map(colorPotValue, 0, 1023, 1, 300);  // Map color sensitivity pot value
  if (millis() - lastPrintTime >= 1000) {
    Serial.print("Threshold: ");
    Serial.print(threshold);
    Serial.print(" | Color Sensitivity: ");
    Serial.println(colorSensitivity);
    lastPrintTime = millis();
  }

  static bool lastButtonState = HIGH;
  bool buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW && lastButtonState == HIGH) {
    mode = (mode + 1) % 9;  // Now 9 modes, button cycles through them
    flashModeColor();
    delay(200);
  }
  lastButtonState = buttonState;

  // Read the audio input
  for (int i = 0; i < SAMPLES; i++) {
    vReal[i] = analogRead(AUDIO_PIN);
    vImag[i] = 0;
    delayMicroseconds(1000);  // Small delay for sampling
  }

  // Perform FFT on the audio signal
  FFT.compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.complexToMagnitude(vReal, vImag, SAMPLES);

  long totalMagnitude = 0;
  for (int i = 0; i < SAMPLES; i++) {
    totalMagnitude += vReal[i];
  }
  int soundLevel = map(totalMagnitude, 0, 1024 * SAMPLES, -500, 255);  // Map sound level to a range

  if (millis() - lastPrintTime >= 1000) {
    Serial.print("Sound Level: ");
    Serial.println(soundLevel);
  }

  // Determine the RGB color based on sound levels and color sensitivity
  int bass = 0, mid = 0, treble = 0;
  for (int i = 0; i < SAMPLES / 3; i++) bass += vReal[i];
  for (int i = SAMPLES / 3; i < 2 * SAMPLES / 3; i++) mid += vReal[i];
  for (int i = 2 * SAMPLES / 3; i < SAMPLES; i++) treble += vReal[i];

  int green = map(bass, 0, 1024 * (SAMPLES / 3), 50, colorSensitivity);
  int red = map(mid, 0, 1024 * (SAMPLES / 3), 50, colorSensitivity);
  int blue = map(treble, 0, 1024 * (SAMPLES / 3), 50, colorSensitivity);

  red = constrain(red + random(-20, 20), 0, 255);
  green = constrain(green + random(-20, 20), 0, 255);
  blue = constrain(blue + random(-20, 20), 0, 255);


  if (soundLevel < threshold) {
    if (mode == 0) {
      // Mode 0: Moving lights
      for (int i = NUM_LEDS - 1; i > 0; i--) leds[i] = leds[i - 1];
      leds[0] = CRGB(red, green, blue);
    }

    // Mode 1: Pulse Effect - New mode
    else if (mode == 1) {
      // Pulse effect that changes intensity and color based on sound level
      int pulseSpeed = map(soundLevel, -500, threshold, 10, 255);
      int pulseValue = abs(sin(millis() / 200.0) * pulseSpeed);  // Smooth pulse

      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB(red, green, blue).fadeToBlackBy(pulseValue);
      }
    }

    // Mode 2: Smooth Wave effect
    else if (mode == 2) {
      for (int i = 0; i < NUM_LEDS; i++) {
        int wavePos = (i + millis() / 50) % NUM_LEDS;                         // Move the wave over time
        int waveColor = (sin(wavePos * 0.2 + millis() / 100.0) * 128 + 127);  // Smooth sine wave
        waveColor = constrain(waveColor, 0, 255);
        leds[i] = CRGB(red, green, blue).lerp8(CRGB::Black, waveColor);
      }
    }

    // Mode 3: Fade effect
    else if (mode == 3) {
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB(red, green, blue).fadeToBlackBy((i * 255) / NUM_LEDS);
      }
    }

    // Mode 4: Ripple effect
    else if (mode == 4) {
      static int ripplePos = 0;
      ripplePos = (ripplePos + 1) % (NUM_LEDS / 2);
      leds[NUM_LEDS / 2 + ripplePos] = CRGB(red, green, blue);
      leds[NUM_LEDS / 2 - ripplePos] = CRGB(red, green, blue);
    }

    // Mode 5: Smooth Color Spectrum effect
    else if (mode == 5) {
      static int hue = 0;
      int speed = map(soundLevel, -500, threshold, 10, 200);  // Adjust speed based on frequency
      hue = (hue + speed) % 255;                              // Update hue dynamically
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(hue + (i * 255 / NUM_LEDS), 255, 255);  // Smooth color transition
      }
    }

    // Mode 6: Red to Yellow Spectrum
    else if (mode == 6) {
      static int hue = 0;
      int speed = map(soundLevel, -500, threshold, 10, 200);  // Adjust speed based on frequency
      hue = (hue + speed) % 43;                               // Update hue for red to yellow range (approx 0 to 43)
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(hue + (i * 43 / NUM_LEDS), 255, 255);  // Transition from red to yellow
      }
    }

    // Mode 7: Blue to Green Spectrum
    else if (mode == 7) {
      static int hue = 170;                                   // Starting from blue
      int speed = map(soundLevel, -500, threshold, 10, 200);  // Adjust speed based on frequency
      hue = (hue + speed) % 85;                               // Update hue for blue to green range (approx 170 to 255)
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(hue + (i * 85 / NUM_LEDS), 255, 255);  // Transition from blue to green
      }
    }

    // Mode 8: Purple to Pink Spectrum
    else if (mode == 8) {
      static int hue = 160;                                   // Starting from purple
      int speed = map(soundLevel, -500, threshold, 10, 200);  // Adjust speed based on frequency
      hue = (hue + speed) % 128;                              // Update hue for purple to pink range (approx 160 to 255)
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(hue + (i * 128 / NUM_LEDS), 255, 255);  // Transition from purple to pink
      }
    }

  } else {
    for (int i = 0; i < NUM_LEDS; i++) leds[i].fadeToBlackBy(40);  // LEDs fade when sound level is low
  }

  FastLED.show();
  delay(50);
}

void flashModeColor() {
  CRGB flashColor;
  switch (mode) {
    case 0: flashColor = CRGB::Red; break;
    case 1: flashColor = CRGB::Green; break;
    case 2: flashColor = CRGB::Blue; break;
    case 3: flashColor = CRGB::Yellow; break;
    case 4: flashColor = CRGB::Purple; break;
    case 5: flashColor = CRGB::Orange; break;
    case 6: flashColor = CRGB::Red; break;     // Red to Yellow
    case 7: flashColor = CRGB::Blue; break;    // Blue to Green
    case 8: flashColor = CRGB::Purple; break;  // Purple to Pink
  }
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = flashColor;
  FastLED.show();
  delay(200);
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Black;
  FastLED.show();
  delay(100);
}