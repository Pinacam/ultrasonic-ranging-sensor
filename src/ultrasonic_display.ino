#include <Arduino.h>

/*
  Display sensor-derived voltage (0–5 V) as X.XX on a
  4-digit, common-anode 7-segment (your pin mapping).
*/

// === CONFIG ===
// Maximum distance (cm) that corresponds to 5.00 V.
// Adjust if your sensor range is different.
const float MAX_DIST = 400.0;

// Flag for your display type
const bool commonAnode = true;

// HC-SR04 pins
const int trigPin = 15;  // A1 on Uno
const int echoPin = 16;  // A2 on Uno

// Segment pins a, b, c, d, e, f, g
// a→6, b→7, c→8, d→9, e→10, f→12, g→11
const int segPins[7] = { 6, 7, 8, 9, 10, 12, 11 };
// DP (decimal point) → pin 13
const int dpPin      = 13;

// Digit-enable cathodes d1→d4 → pins 2,3,4,5
const int digPins[4] = { 2, 3, 4, 5 };

// 0–9 bit patterns (gfedcba)
const byte DIGIT_MAP[10] = {
  B0111111, // 0
  B0000110, // 1
  B1011011, // 2
  B1001111, // 3
  B1100110, // 4
  B1101101, // 5
  B1111101, // 6
  B0000111, // 7
  B1111111, // 8
  B1101111  // 9
};

void setup() {
  // HC-SR04
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Segments + DP
  for (int i = 0; i < 7; i++)  pinMode(segPins[i], OUTPUT);
  pinMode(dpPin, OUTPUT);
  digitalWrite(dpPin, commonAnode ? HIGH : LOW);

  // Digit cathodes
  for (int d = 0; d < 4; d++)  pinMode(digPins[d], OUTPUT);
}

// send pulse & read echo → distance in cm
float measureDistanceCm() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  return duration / 58.0;
}

// digits[] holds four values 0–9 or -1 for blank.
// dpPos is the index (0–3) of the digit that should have its DP lit.
void displayDigits(int digits[4], int dpPos) {
  for (int pos = 0; pos < 4; pos++) {
    // 1) turn all digits OFF
    for (int d = 0; d < 4; d++)
      digitalWrite(digPins[d], commonAnode ? LOW : HIGH);

    // 2) set DP for this digit
    bool dpOn = (pos == dpPos);
    digitalWrite(dpPin, commonAnode ? !dpOn : dpOn);

    // 3) set segments for digits[pos]
    byte pattern = (digits[pos] >= 0 && digits[pos] < 10)
                   ? DIGIT_MAP[digits[pos]]
                   : 0; // blank
    for (int s = 0; s < 7; s++) {
      bool segOn = (pattern >> s) & 1;
      digitalWrite(segPins[s],
                   commonAnode ? !segOn : segOn);
    }

    // 4) enable this digit
    digitalWrite(digPins[pos], commonAnode ? HIGH : LOW);
    delay(4);
  }
}

void loop() {
  // 1) measure distance and map to 0–5 V
  float dist  = measureDistanceCm();
  float volts = constrain(dist / MAX_DIST * 5.0, 0.0, 5.0);

  // 2) convert to hundredths (0–500)
  int v100 = int(volts * 100 + 0.5);

  // 3) split into X.XX
  int d0 = v100 / 100;        // integer part (0–5)
  int d1 = (v100 / 10) % 10;  // first decimal
  int d2 = v100 % 10;         // second decimal

  int digits[4] = { d0, d1, d2, -1 };  // last digit blank
  int dpPos = 0;                       // light DP on digit 0 → X.XX

  // 4) display!
  displayDigits(digits, dpPos);
}



