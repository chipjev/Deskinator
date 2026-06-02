//  Cleaning Robot — Arduino UNO
//  APDS-9960 gesture sensor — wave LEFT→RIGHT to start
//  Priority Edge Logic: Front → Right → Left
//  LED OFF = idle | LED ON = cleaning | LED BLINK = done


// Libraries
#include <Wire.h>
#include <SparkFun_APDS9960.h>

// Pin definitions
#define LED_PIN        13

#define TRIG_FRONT     A3
#define ECHO_FRONT     A2
#define TRIG_LEFT      4
#define ECHO_LEFT      12
#define TRIG_RIGHT     A1
#define ECHO_RIGHT     A0

#define IN1            9
#define IN2            8
#define IN3            6
#define IN4            5
#define ENA            10
#define ENB            3

// Tunable constants
#define MOTOR_SPEED       180
#define TURN_SPEED        160
#define EDGE_DIST_CM      40
#define CLEAN_DURATION_MS 120000UL
#define BLINK_INTERVAL    300

// Globals
bool running          = false;
bool blinking         = false;
bool ledState         = false;
unsigned long startTime  = 0;
unsigned long lastBlink  = 0;

SparkFun_APDS9960 apds;

// Setup
void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(A5));

  // Motor pins
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);
  motorsStop();

  // LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);        // OFF at startup

  // Ultrasonic sensor pins
  pinMode(TRIG_FRONT, OUTPUT); pinMode(ECHO_FRONT, INPUT);
  pinMode(TRIG_LEFT,  OUTPUT); pinMode(ECHO_LEFT,  INPUT);
  pinMode(TRIG_RIGHT, OUTPUT); pinMode(ECHO_RIGHT, INPUT);

  // Gesture sensor init
  Wire.begin();
  if (!apds.init()) {
    Serial.println("APDS-9960 init failed! Check wiring.");
    while (1);
  }

  apds.setLEDDrive(LED_DRIVE_100MA);
  apds.setGestureGain(GGAIN_4X);
  apds.setProximityGain(PGAIN_4X);

  if (!apds.enableGestureSensor(true)) {
    Serial.println("Gesture sensor enable failed!");
    while (1);
  }

  Serial.println("Ready. LED OFF.");
  Serial.println("Wave LEFT → RIGHT to start cleaning.");
}

// Main loop
void loop() {

  // STATE 1: Blinking (cycle complete)
  if (blinking) {
    // Non-blocking blink
    if (millis() - lastBlink >= BLINK_INTERVAL) {
      lastBlink = millis();
      ledState  = !ledState;
      digitalWrite(LED_PIN, ledState ? HIGH : LOW);
    }

    // Still listening for gesture to restart
    if (apds.isGestureAvailable()) {
      int gesture = apds.readGesture();
      if (gesture == DIR_RIGHT) {
        Serial.println("Restarting cleaning cycle...");
        blinking = false;
        digitalWrite(LED_PIN, HIGH);
        startCleaning();
      }
    }
    return;
  }

  // STATE 2: Idle (waiting for gesture)
  if (!running) {
    if (apds.isGestureAvailable()) {
      int gesture = apds.readGesture();
      if (gesture == DIR_RIGHT) {
        Serial.println("Left→Right gesture detected! Starting...");
        digitalWrite(LED_PIN, HIGH);   // Solid ON = running
        startCleaning();
      } else {
        Serial.println("Wrong gesture — wave LEFT to RIGHT to start.");
      }
    }
    return;
  }

  // STATE 3: Cleaning

  // Check if 2 minute cycle is done
  if (millis() - startTime >= CLEAN_DURATION_MS) {
    finishCleaning();
    return;
  }

  // STEP 1: Read front sensor
  long distFront = getDistance(TRIG_FRONT, ECHO_FRONT);
  bool edgeFront = (distFront > EDGE_DIST_CM);

  Serial.print("F:"); Serial.print(distFront);

  if (!edgeFront) {
    Serial.println(" → Clear, moving forward.");
    motorsForward();
    delay(50);
    return;
  }

  // STEP 2: Front edge → back up, check RIGHT
  motorsBackward();
  delay(4000);
  motorsStop();
  delay(100);

  long distRight = getDistance(TRIG_RIGHT, ECHO_RIGHT);
  bool edgeRight = (distRight > EDGE_DIST_CM);

  Serial.print(" R:"); Serial.print(distRight);

  if (!edgeRight) {
    Serial.println(" → Front edge, Right clear → Turning Right");
    turnRight(600);
    delay(50);
    return;
  }

  // STEP 3: Right also edge → check LEFT
  long distLeft = getDistance(TRIG_LEFT, ECHO_LEFT);
  bool edgeLeft = (distLeft > EDGE_DIST_CM);

  Serial.print(" L:"); Serial.print(distLeft);

  if (!edgeLeft) {
    Serial.println(" → Front+Right edge, Left clear → Turning Left");
    turnLeft(600);
  } else {
    Serial.println(" → Trapped on all sides! U-Turn 180°");
    turnRight(1000);
  }

  delay(50);
}

// State helpers
void startCleaning() {
  running   = true;
  startTime = millis();
  motorsForward();
  Serial.println("Cleaning started. LED solid ON.");
}

void finishCleaning() {
  running   = false;
  blinking  = true;
  lastBlink = millis();
  motorsStop();
  Serial.println("Cleaning done! LED blinking.");
  Serial.println("Wave LEFT → RIGHT to start again.");
}

// Distance measurement (HC-SR04)
long getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 25000);
  if (duration == 0) return 999;
  return duration * 0.034 / 2;
}

// Motor control
void motorsForward() {
  analogWrite(ENA, MOTOR_SPEED);
  analogWrite(ENB, MOTOR_SPEED);
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
}

void motorsBackward() {
  analogWrite(ENA, MOTOR_SPEED);
  analogWrite(ENB, MOTOR_SPEED);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void motorsStop() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

void turnRight(int durationMs) {
  analogWrite(ENA, TURN_SPEED);
  analogWrite(ENB, TURN_SPEED);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
  delay(durationMs);
  motorsForward();
}

void turnLeft(int durationMs) {
  analogWrite(ENA, TURN_SPEED);
  analogWrite(ENB, TURN_SPEED);
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  delay(durationMs);
  motorsForward();
}

// LED blink (blocking — used nowhere now, kept for reference)
void blinkLED(int times, int intervalMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(intervalMs);
    digitalWrite(LED_PIN, LOW);
    delay(intervalMs);
  }
}
