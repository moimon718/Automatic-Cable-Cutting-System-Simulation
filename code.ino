//c++ code
#include <Adafruit_LiquidCrystal.h>
#include <Keypad.h>
#include <Servo.h>

// === LCD ===
Adafruit_LiquidCrystal lcd_1(0);  // For Tinkercad I2C LCD block

// === Ultrasonic Sensor ===
#define TRIG_PIN 2
#define ECHO_PIN 3

// === Servo Motor ===
Servo cutter;
#define SERVO_PIN 4

// === DC Motor via L293D ===
#define ENA A0
#define IN1 A1
#define IN2 A2

// === Keypad ===
const byte ROWS = 4; 
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {5, 6, 7, 8}; // R1-R4
byte colPins[COLS] = {9, 10, 11, 12}; // C1-C4

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// === Push Button to Read Distance ===
#define BUTTON_PIN 13

// === Slide Switch ===
#define SLIDE_SWITCH_PIN A3

// === Variables ===
String inputLength = "";
String inputPieces = "";
long targetDistance = 0;
int totalPieces = 0;
bool enteringLength = true;

void cutCable() {
  cutter.write(85);   // Rotate to cut
  delay(600);
  cutter.write(0);    // Back to original
  delay(600);
}

long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  long distance = duration * 0.034 / 2;
  return distance;
}

void moveUntilTarget(long target) {
  // Start motor
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 180);  // Adjust speed as needed

  while (getDistance() < target) {
    delay(50);
  }

  // Stop motor
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}

void setup() {
  // LCD
  lcd_1.begin(16, 2);
  lcd_1.print("Enter length:");

  // Servo
  cutter.attach(SERVO_PIN);

  // Motor pins
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  // Ultrasonic
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Button
  pinMode(BUTTON_PIN, INPUT);

  // Slide switch
  pinMode(SLIDE_SWITCH_PIN, INPUT);

  // Initial motor state
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}

void loop() {
  char key = keypad.getKey();

  if (key) {
    if (key >= '0' && key <= '9') {
      if (enteringLength) {
        inputLength += key;
        lcd_1.setCursor(0, 1);
        lcd_1.print("Length: " + inputLength + " cm ");
      } else {
        inputPieces += key;
        lcd_1.setCursor(0, 1);
        lcd_1.print("Pieces: " + inputPieces + "    ");
      }
    } 
    else if (key == 'A') {
      // Switch to piece input
      enteringLength = false;
      lcd_1.clear();
      lcd_1.setCursor(0, 0);
      lcd_1.print("Enter pieces:");
      lcd_1.setCursor(0, 1);
      inputPieces = "";
    } 
    else if (key == '*') {
      // Reset current input
      if (enteringLength) {
        inputLength = "";
        lcd_1.setCursor(0, 1);
        lcd_1.print("                ");
      } else {
        inputPieces = "";
        lcd_1.setCursor(0, 1);
        lcd_1.print("                ");
      }
    } 
    else if (key == '#') {
      if (digitalRead(SLIDE_SWITCH_PIN) == HIGH) {
        targetDistance = inputLength.toInt();
        totalPieces = inputPieces.toInt();

        lcd_1.clear();
        lcd_1.setCursor(0, 0);
        lcd_1.print("Cutting...");

        for (int i = 1; i <= totalPieces; i++) {
          moveUntilTarget(targetDistance);
          cutCable();

          lcd_1.setCursor(0, 1);
          lcd_1.print("Piece " + String(i));
          delay(1000);
        }

        lcd_1.clear();
        lcd_1.print("All Done!");
        delay(2000);
      } else {
        lcd_1.clear();
        lcd_1.print("Switch is OFF");
        lcd_1.setCursor(0, 1);
        lcd_1.print("Enable to cut");
        delay(2000);
      }

      // Reset for next run
      inputLength = "";
      inputPieces = "";
      enteringLength = true;
      lcd_1.clear();
      lcd_1.print("Enter length:");
    }
  }

  if (digitalRead(BUTTON_PIN) == HIGH) {
    long currentDistance = getDistance();
    lcd_1.setCursor(0, 1);
    lcd_1.print("Now: " + String(currentDistance) + " cm ");
    delay(1000);
  }
}
