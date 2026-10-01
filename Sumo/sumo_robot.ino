// ============================================================================
// Besomi Academy 2026
// Sumo Robot Educational Kit
// ----------------------------------------------------------------------------
// Version      : v1
// Platform     : Arduino UNO Q
// Compatibility: Arduino App Lab
//
// Description:
// Educational autonomous Sumo Robot control software designed for robotics
// training, STEM education, and beginner-to-intermediate robot competitions.
//
// Features:
// - Push-button start system with 5-second competition delay
// - Opponent detection using obstacle sensors
// - Arena boundary detection using line sensors
// - Autonomous search and attack behavior
// - Real-time serial debugging output
// - Motor driver control for differential drive robots
//
// Robot Behavior:
// 1. Waits for the start button.
// 2. Executes a 5-second competition countdown.
// 3. Searches for an opponent.
// 4. Attacks when an opponent is detected.
// 5. Escapes when the arena boundary is detected.
// 6. Returns to standby mode when stopped.
//
// Copyright (c) 2026 Besomi Academy.
// All Rights Reserved.
//
// This software is provided as part of the Besomi Academy
// Sumo Robot Educational Kit and is intended for educational,
// training, and competition purposes.
//
// Author : Besomi Academy
// Year   : 2026
// ============================================================================

#include <Arduino_RouterBridge.h> // UNO Q internal routing bridge
#define Serial Monitor            // Redirect Serial output to USB-C/App Lab

// ===================== HARDWARE PIN DEFINITIONS =====================

// Status LEDs
const int RED_LED_PIN = 11;
const int YELLOW_LED_PIN = 13;
const int GREEN_LED_PIN = 12;

// LEFT MOTOR DRIVER CONNECTIONS
const int LEFTMOTOR_REN_PIN = 2;   // Blue wire
const int LEFTMOTOR_RPWM_PIN = 3;  // Purple wire
const int LEFTMOTOR_LEN_PIN = 4;   // Green wire
const int LEFTMOTOR_LPWM_PIN = 5;  // White wire

// RIGHT MOTOR DRIVER CONNECTIONS
const int RIGHTMOTOR_REN_PIN = 7;   // Blue wire
const int RIGHTMOTOR_RPWM_PIN = 9;  // Purple wire
const int RIGHTMOTOR_LEN_PIN = 8;   // Green wire
const int RIGHTMOTOR_LPWM_PIN = 6;  // White wire

// INPUT DEVICES
const int SWITCH_PIN = A0;

// Opponent detection sensors
const int OB_LEFT_PIN = A3;
const int OB_CENTER_PIN = A2;
const int OB_RIGHT_PIN = A1;

// Arena boundary detection sensors
const int LINE_LEFT_PIN = A5;
const int LINE_RIGHT_PIN = A4;


// ===================== ROBOT STATES & CONSTANTS =====================

// Obstacle sensor states
const int DETECTED = 0;
const int CLEAR = 1;

// Line sensor states
const int WHITE = 0;
const int BLACK = 1;


// ===================== GLOBAL VARIABLES =====================

// Start switch status
bool switchPressed = false;

// Robot operation status
bool robotStarted = false;

// Sensor readings
int obstacleLeft, obstacleCenter, obstacleRight;
int lineLeft, lineRight;

// Last known opponent direction
// 0 = left, 1 = right
int lastDirection = 1;


// ===================== MOTOR CONTROL FUNCTIONS =====================

// Enable all motor driver channels
void enableMotors() {
  digitalWrite(LEFTMOTOR_REN_PIN, HIGH);
  digitalWrite(LEFTMOTOR_LEN_PIN, HIGH);
  digitalWrite(RIGHTMOTOR_REN_PIN, HIGH);
  digitalWrite(RIGHTMOTOR_LEN_PIN, HIGH);
}

// Immediately stop both drive motors
void stopMotors() {
  digitalWrite(LEFTMOTOR_LPWM_PIN, LOW);
  digitalWrite(LEFTMOTOR_RPWM_PIN, LOW);
  digitalWrite(RIGHTMOTOR_LPWM_PIN, LOW);
  digitalWrite(RIGHTMOTOR_RPWM_PIN, LOW);
}

// Move robot forward
void forward() {
  digitalWrite(LEFTMOTOR_LPWM_PIN, HIGH);
  digitalWrite(LEFTMOTOR_RPWM_PIN, LOW);
  digitalWrite(RIGHTMOTOR_LPWM_PIN, HIGH);
  digitalWrite(RIGHTMOTOR_RPWM_PIN, LOW);
}

// Move robot backward
void backward() {
  digitalWrite(LEFTMOTOR_LPWM_PIN, LOW);
  digitalWrite(LEFTMOTOR_RPWM_PIN, HIGH);
  digitalWrite(RIGHTMOTOR_LPWM_PIN, LOW);
  digitalWrite(RIGHTMOTOR_RPWM_PIN, HIGH);
}

// Rotate robot toward the left side
void left() {
  digitalWrite(LEFTMOTOR_LPWM_PIN, LOW);
  digitalWrite(LEFTMOTOR_RPWM_PIN, HIGH);
  digitalWrite(RIGHTMOTOR_LPWM_PIN, HIGH);
  digitalWrite(RIGHTMOTOR_RPWM_PIN, LOW);
}

// Rotate robot toward the right side
void right() {
  digitalWrite(LEFTMOTOR_LPWM_PIN, HIGH);
  digitalWrite(LEFTMOTOR_RPWM_PIN, LOW);
  digitalWrite(RIGHTMOTOR_LPWM_PIN, LOW);
  digitalWrite(RIGHTMOTOR_RPWM_PIN, HIGH);
}


// ===================== SENSOR ACQUISITION =====================

// Read all sensors and update robot state variables
void readSensors() {

  switchPressed = (digitalRead(SWITCH_PIN) == LOW);

  obstacleLeft   = (digitalRead(OB_LEFT_PIN) == LOW) ? DETECTED : CLEAR;
  obstacleCenter = (digitalRead(OB_CENTER_PIN) == LOW) ? DETECTED : CLEAR;
  obstacleRight  = (digitalRead(OB_RIGHT_PIN) == LOW) ? DETECTED : CLEAR;

  lineLeft  = (digitalRead(LINE_LEFT_PIN) == LOW) ? WHITE : BLACK;
  lineRight = (digitalRead(LINE_RIGHT_PIN) == LOW) ? WHITE : BLACK;
}


// ===================== SERIAL DEBUG MONITOR =====================

// Print current robot status to the serial monitor
void printDebug() {

  Serial.print("SW: ");
  Serial.print(switchPressed ? "ON" : "OFF");

  Serial.print(" | OB [L C R]: ");
  Serial.print(obstacleLeft == DETECTED ? "DET" : "CLR");
  Serial.print(" ");
  Serial.print(obstacleCenter == DETECTED ? "DET" : "CLR");
  Serial.print(" ");
  Serial.print(obstacleRight == DETECTED ? "DET" : "CLR");

  Serial.print(" | LINE [L R]: ");
  Serial.print(lineLeft == BLACK ? "BLK" : "WHT");
  Serial.print(" ");
  Serial.print(lineRight == BLACK ? "BLK" : "WHT");

  Serial.print(" | DIR: ");
  Serial.println(lastDirection == 0 ? "LEFT" : "RIGHT");
}


// ===================== STARTUP & COMPETITION SEQUENCE =====================

// Wait for the operator start command and perform countdown
void waitForStart() {

  robotStarted = false;

  // Robot standby indication
  digitalWrite(RED_LED_PIN, HIGH);
  digitalWrite(YELLOW_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);

  stopMotors();

  Serial.println("Waiting for start...");

  // Wait for switch release
  while (switchPressed == true) {
    readSensors();
    printDebug();
  }

  // Wait for switch press
  while (switchPressed == false) {
    readSensors();
    printDebug();
  }

  // Competition countdown
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(YELLOW_LED_PIN, HIGH);
  digitalWrite(GREEN_LED_PIN, LOW);

  Serial.println("Starting in 5 seconds...");
  delay(5000);

  // Robot active indication
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(YELLOW_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, HIGH);

  robotStarted = true;
}


// ===================== ARDUINO SETUP =====================

// Arduino initialization routine
void setup() {

  // Initialize the UNO Q hardware bridge
  Bridge.begin();

  // Initialize serial communication
  Serial.begin(9600);

  // Allow App Lab connection to initialize
  delay(1500);

  // LED outputs
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  // Left motor driver outputs
  pinMode(LEFTMOTOR_REN_PIN, OUTPUT);
  pinMode(LEFTMOTOR_RPWM_PIN, OUTPUT);
  pinMode(LEFTMOTOR_LEN_PIN, OUTPUT);
  pinMode(LEFTMOTOR_LPWM_PIN, OUTPUT);

  // Right motor driver outputs
  pinMode(RIGHTMOTOR_REN_PIN, OUTPUT);
  pinMode(RIGHTMOTOR_RPWM_PIN, OUTPUT);
  pinMode(RIGHTMOTOR_LEN_PIN, OUTPUT);
  pinMode(RIGHTMOTOR_LPWM_PIN, OUTPUT);

  // Start switch input
  pinMode(SWITCH_PIN, INPUT_PULLUP);

  // Opponent sensors
  pinMode(OB_LEFT_PIN, INPUT);
  pinMode(OB_CENTER_PIN, INPUT);
  pinMode(OB_RIGHT_PIN, INPUT);

  // Line sensors
  pinMode(LINE_LEFT_PIN, INPUT);
  pinMode(LINE_RIGHT_PIN, INPUT);

  // Enable motor drivers
  enableMotors();

  // Enter startup sequence
  waitForStart();
}


// ===================== MAIN ROBOT LOGIC =====================

// Main autonomous robot control loop
void loop() {

  readSensors();
  printDebug();

  // STOP MODE
  // Return robot to standby when switch is released
  if (switchPressed == false) {

    stopMotors();

    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, LOW);

    waitForStart();
    return;
  }

  // LINE ESCAPE
  // Retreat from arena boundary to avoid ring-out
  if (lineLeft == WHITE || lineRight == WHITE) {

    Serial.println("ACTION: LINE ESCAPE");

    backward();
    delay(600);

    left();
    delay(600);
  }

  // ATTACK MODE
  // Move forward when opponent is directly ahead
  else if (obstacleCenter == DETECTED) {

    Serial.println("ACTION: ATTACK");
    forward();
  }

  // TARGET DETECTED ON LEFT SIDE
  else if (obstacleLeft == DETECTED) {

    Serial.println("ACTION: LEFT TARGET");

    lastDirection = 0;
    left();
  }

  // TARGET DETECTED ON RIGHT SIDE
  else if (obstacleRight == DETECTED) {

    Serial.println("ACTION: RIGHT TARGET");

    lastDirection = 1;
    right();
  }

  // SEARCH MODE
  // Continue moving while looking for an opponent
  else {

    Serial.println("ACTION: SEARCH");
    forward();
  }
}
