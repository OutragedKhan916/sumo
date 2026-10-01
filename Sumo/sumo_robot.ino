// ============================================================================
// BESOMI ACADEMY 2026
// PROACTIVE SUMO ROBOT CONTROLLER
// Arduino UNO Q
//
// MOTOR DRIVER:
// Pololu Dual G2 High-Power Motor Driver configuration
//
// IMPORTANT:
// This program controls four motor channels directly using DIR + PWM.
// The Pololu shield supports remapped control pins.
// ============================================================================

#include <Arduino_RouterBridge.h>

// ============================================================================
// MOTOR PINS
// ============================================================================

// LEFT SIDE
const int M1_DIR = D7;
const int M1_PWM = D9;

const int M2_DIR = D8;
const int M2_PWM = D10;

// RIGHT SIDE
const int M3_DIR = D12;
const int M3_PWM = D5;

const int M4_DIR = D13;
const int M4_PWM = D6;


// ============================================================================
// OBSTACLE SENSORS
// ============================================================================

const int OB_LEFT_PIN        = D0;
const int OB_RIGHT_PIN       = D1;

const int OB_BACK_RIGHT_PIN  = D2;
const int OB_BACK_LEFT_PIN   = D3;

const int OB_FRONT_RIGHT_PIN = D4;
const int OB_FRONT_LEFT_PIN  = D11;


// ============================================================================
// LINE SENSORS
// ============================================================================

const int LINE_FRONT_LEFT_PIN  = A0;
const int LINE_FRONT_RIGHT_PIN = A1;

const int LINE_BACK_LEFT_PIN   = A4;
const int LINE_BACK_RIGHT_PIN  = A5;


// ============================================================================
// ULTRASONIC SENSOR
// ============================================================================
//
// Your connection list gives A2 as "Ultrasonic Signal".
//
// This code assumes a single-wire ultrasonic sensor where the same pin
// is used for trigger and echo.
//
// If your sensor is HC-SR04, DO NOT use this configuration.
// HC-SR04 normally requires separate TRIG and ECHO pins.
//

const int ULTRASONIC_PIN = A2;


// ============================================================================
// START
// ============================================================================
//
// Your listed wiring has no separate start-button pin.
//
// Therefore the robot automatically waits 5 seconds after power/reset.
//
// If your competition has an external start system, we can add it once
// you provide its exact pin.
//

const unsigned long START_DELAY = 5000;


// ============================================================================
// MOTOR DIRECTION
// ============================================================================
//
// Test the robot with its wheels lifted.
//
// When positive speed is commanded, all four wheels should produce
// forward motion.
//
// Change these if necessary.
//

bool INVERT_M1 = false;
bool INVERT_M2 = false;

bool INVERT_M3 = true;
bool INVERT_M4 = true;


// ============================================================================
// SPEED SETTINGS
// ============================================================================

const int SEARCH_SPEED = 105;

const int TRACK_SPEED = 155;

const int TURN_SPEED = 190;

const int ATTACK_SPEED = 255;

const int DEFENSE_SPEED = 210;

const int ESCAPE_SPEED = 255;

const int INTERCEPT_SPEED = 220;


// ============================================================================
// SENSOR LOGIC
// ============================================================================

const int SENSOR_DETECTED = LOW;

const int LINE_DETECTED = LOW;


// ============================================================================
// ULTRASONIC SETTINGS
// ============================================================================

const float SOUND_SPEED_CM_PER_US = 0.0343;

// Maximum useful distance for Sumo
const float MAX_TARGET_DISTANCE = 250.0;

// Ignore readings closer than this because they may be invalid
const float MIN_TARGET_DISTANCE = 3.0;

// Ultrasonic update period
const unsigned long ULTRASONIC_INTERVAL = 30;

// Pulse timeout
const unsigned long ULTRASONIC_TIMEOUT = 15000;


// ============================================================================
// CLOSING SPEED
// ============================================================================
//
// Positive closing speed means the opponent is getting closer.
//
// Example:
//
// Previous distance = 100 cm
// Current distance  = 90 cm
// Time              = 0.1 s
//
// Closing speed = (100 - 90) / 0.1
//               = 100 cm/s
//

float ultrasonicDistance = -1.0;

float previousDistance = -1.0;

float closingSpeed = 0.0;

float previousClosingSpeed = 0.0;

float closingAcceleration = 0.0;

unsigned long previousDistanceTime = 0;

unsigned long lastUltrasonicRead = 0;


// ============================================================================
// ULTRASONIC VALIDITY
// ============================================================================

bool ultrasonicValid = false;


// ============================================================================
// OPPONENT DIRECTION
// ============================================================================

enum OpponentDirection {

  OPP_NONE,

  OPP_LEFT,

  OPP_FRONT,

  OPP_RIGHT,

  OPP_BACK

};


OpponentDirection opponentDirection = OPP_NONE;


// ============================================================================
// OPPONENT MOTION
// ============================================================================

enum OpponentMotion {

  MOTION_UNKNOWN,

  MOTION_APPROACHING_FAST,

  MOTION_APPROACHING_SLOW,

  MOTION_STATIONARY,

  MOTION_MOVING_AWAY,

  MOTION_CROSSING

};


OpponentMotion opponentMotion = MOTION_UNKNOWN;


// ============================================================================
// PREVIOUS IR SENSOR STATE
// ============================================================================

bool previousFL = false;
bool previousFR = false;

bool previousL = false;
bool previousR = false;

bool previousBL = false;
bool previousBR = false;


// ============================================================================
// OPPONENT TRACKING
// ============================================================================

unsigned long lastOpponentSeen = 0;

unsigned long lastPatternChange = 0;

unsigned long lastTrackingUpdate = 0;


// ============================================================================
// BOUNDARY ESCAPE STATE
// ============================================================================

enum EscapeState {

  ESCAPE_NONE,

  ESCAPE_BACK_FROM_FRONT,

  ESCAPE_TURN_FROM_FRONT_LEFT,

  ESCAPE_TURN_FROM_FRONT_RIGHT,

  ESCAPE_FORWARD_FROM_BACK,

  ESCAPE_TURN_FROM_BACK_LEFT,

  ESCAPE_TURN_FROM_BACK_RIGHT,

  ESCAPE_TURN_FROM_FRONT,

  ESCAPE_TURN_FROM_BACK

};


EscapeState escapeState = ESCAPE_NONE;

unsigned long escapeStarted = 0;

unsigned long escapeDuration = 0;


// ============================================================================
// ROBOT STATE
// ============================================================================

bool robotStarted = false;

unsigned long robotStartTime = 0;


// ============================================================================
// MOTOR CONTROL
// ============================================================================

void setMotor(
  int dirPin,
  int pwmPin,
  int speed,
  bool invert
) {

  speed = constrain(speed, -255, 255);

  if (invert) {

    speed = -speed;

  }


  if (speed > 0) {

    digitalWrite(dirPin, HIGH);

    analogWrite(pwmPin, speed);

  }

  else if (speed < 0) {

    digitalWrite(dirPin, LOW);

    analogWrite(pwmPin, -speed);

  }

  else {

    analogWrite(pwmPin, 0);

  }

}


// ============================================================================
// DIFFERENTIAL DRIVE
// ============================================================================

void drive(int leftSpeed, int rightSpeed) {

  leftSpeed = constrain(leftSpeed, -255, 255);

  rightSpeed = constrain(rightSpeed, -255, 255);


  setMotor(
    M1_DIR,
    M1_PWM,
    leftSpeed,
    INVERT_M1
  );


  setMotor(
    M2_DIR,
    M2_PWM,
    leftSpeed,
    INVERT_M2
  );


  setMotor(
    M3_DIR,
    M3_PWM,
    rightSpeed,
    INVERT_M3
  );


  setMotor(
    M4_DIR,
    M4_PWM,
    rightSpeed,
    INVERT_M4
  );

}


// ============================================================================
// MOVEMENT
// ============================================================================

void stopMotors() {

  drive(0, 0);

}


void forward(int speed) {

  drive(speed, speed);

}


void backward(int speed) {

  drive(-speed, -speed);

}


void rotateLeft(int speed) {

  drive(-speed, speed);

}


void rotateRight(int speed) {

  drive(speed, -speed);

}


// ============================================================================
// IR SENSOR FUNCTIONS
// ============================================================================

bool FL() {

  return digitalRead(OB_FRONT_LEFT_PIN) == SENSOR_DETECTED;

}


bool FR() {

  return digitalRead(OB_FRONT_RIGHT_PIN) == SENSOR_DETECTED;

}


bool L() {

  return digitalRead(OB_LEFT_PIN) == SENSOR_DETECTED;

}


bool R() {

  return digitalRead(OB_RIGHT_PIN) == SENSOR_DETECTED;

}


bool BL() {

  return digitalRead(OB_BACK_LEFT_PIN) == SENSOR_DETECTED;

}


bool BR() {

  return digitalRead(OB_BACK_RIGHT_PIN) == SENSOR_DETECTED;

}


// ============================================================================
// OPPONENT DETECTION
// ============================================================================

bool opponentDetected() {

  return FL() ||
         FR() ||
         L()  ||
         R()  ||
         BL() ||
         BR();

}


// ============================================================================
// OPPONENT DIRECTION
// ============================================================================

OpponentDirection getOpponentDirection() {

  bool fl = FL();
  bool fr = FR();

  bool l = L();
  bool r = R();

  bool bl = BL();
  bool br = BR();


  // Strongest front indication

  if (fl && fr) {

    return OPP_FRONT;

  }


  if (fl) {

    return OPP_LEFT;

  }


  if (fr) {

    return OPP_RIGHT;

  }


  if (l) {

    return OPP_LEFT;

  }


  if (r) {

    return OPP_RIGHT;

  }


  if (bl || br) {

    return OPP_BACK;

  }


  return OPP_NONE;

}


// ============================================================================
// SINGLE PIN ULTRASONIC
// ============================================================================
//
// This is for sensors with one shared signal pin.
//
// The sensor is briefly driven HIGH to trigger.
// Then the same pin becomes an input.
// The returning pulse is measured with pulseIn().
//

float readUltrasonic() {

  pinMode(ULTRASONIC_PIN, OUTPUT);

  digitalWrite(ULTRASONIC_PIN, LOW);

  delayMicroseconds(3);

  digitalWrite(ULTRASONIC_PIN, HIGH);

  delayMicroseconds(10);

  digitalWrite(ULTRASONIC_PIN, LOW);


  pinMode(ULTRASONIC_PIN, INPUT);


  unsigned long duration = pulseIn(
    ULTRASONIC_PIN,
    HIGH,
    ULTRASONIC_TIMEOUT
  );


  if (duration == 0) {

    return -1.0;

  }


  float distance =
    (duration * SOUND_SPEED_CM_PER_US) / 2.0;


  if (distance < MIN_TARGET_DISTANCE ||
      distance > MAX_TARGET_DISTANCE) {

    return -1.0;

  }


  return distance;

}


// ============================================================================
// ULTRASONIC UPDATE
// ============================================================================

void updateUltrasonic() {

  unsigned long now = millis();


  if (now - lastUltrasonicRead <
      ULTRASONIC_INTERVAL) {

    return;

  }


  lastUltrasonicRead = now;


  float newDistance = readUltrasonic();


  if (newDistance < 0) {

    ultrasonicValid = false;

    return;

  }


  ultrasonicValid = true;


  if (previousDistance > 0) {

    unsigned long dt =
      now - previousDistanceTime;


    if (dt > 0) {

      float instantClosingSpeed =
        (previousDistance - newDistance)
        /
        (dt / 1000.0);


      // Low-pass filter.
      // This prevents one noisy reading from changing tactics.

      closingSpeed =
        (closingSpeed * 0.65)
        +
        (instantClosingSpeed * 0.35);


      float instantAcceleration =
        (closingSpeed - previousClosingSpeed)
        /
        (dt / 1000.0);


      closingAcceleration =
        (closingAcceleration * 0.7)
        +
        (instantAcceleration * 0.3);


      previousClosingSpeed = closingSpeed;

    }

  }


  previousDistance = newDistance;

  previousDistanceTime = now;

  ultrasonicDistance = newDistance;

}


// ============================================================================
// OPPONENT MOTION CLASSIFICATION
// ============================================================================
//
// Thresholds are in cm/s.
//
// Positive = approaching
// Negative = moving away
//
// These are initial competition values.
// They should be tuned after testing your actual sensor.
//

void classifyOpponentMotion() {

  if (!opponentDetected()) {

    opponentMotion = MOTION_UNKNOWN;

    return;

  }


  // ----------------------------------------------------------
  // ULTRASONIC HAS PRIORITY WHEN VALID
  // ----------------------------------------------------------

  if (ultrasonicValid) {

    // Very fast closing opponent

    if (closingSpeed >= 80.0) {

      opponentMotion =
        MOTION_APPROACHING_FAST;

      return;

    }


    // Moderate or slow closing

    if (closingSpeed >= 15.0) {

      opponentMotion =
        MOTION_APPROACHING_SLOW;

      return;

    }


    // Moving away

    if (closingSpeed <= -20.0) {

      opponentMotion =
        MOTION_MOVING_AWAY;

      return;

    }


    // Nearly stationary

    if (abs(closingSpeed) < 15.0) {

      opponentMotion =
        MOTION_STATIONARY;

      return;

    }

  }


  // ----------------------------------------------------------
  // FALLBACK TO IR SENSOR MOVEMENT
  // ----------------------------------------------------------

  bool fl = FL();
  bool fr = FR();

  bool l = L();
  bool r = R();

  bool bl = BL();
  bool br = BR();


  bool previousSide =
    previousFL ||
    previousFR ||
    previousL ||
    previousR;


  bool currentFront =
    fl ||
    fr;


  bool previousRear =
    previousBL ||
    previousBR;


  if (previousRear && currentFront) {

    opponentMotion =
      MOTION_APPROACHING_FAST;

    return;

  }


  if (previousSide && currentFront) {

    opponentMotion =
      MOTION_APPROACHING_SLOW;

    return;

  }


  if ((previousFL && r) ||
      (previousFR && l) ||
      (previousL && fr) ||
      (previousR && fl)) {

    opponentMotion =
      MOTION_CROSSING;

    return;

  }


  opponentMotion =
    MOTION_STATIONARY;

}


// ============================================================================
// UPDATE OPPONENT TRACKING
// ============================================================================

void updateOpponentTracking() {

  unsigned long now = millis();


  bool fl = FL();
  bool fr = FR();

  bool l = L();
  bool r = R();

  bool bl = BL();
  bool br = BR();


  if (opponentDetected()) {

    lastOpponentSeen = now;

  }


  bool patternChanged =
    (fl != previousFL) ||
    (fr != previousFR) ||
    (l  != previousL)  ||
    (r  != previousR)  ||
    (bl != previousBL) ||
    (br != previousBR);


  if (patternChanged) {

    lastPatternChange = now;

  }


  previousFL = fl;
  previousFR = fr;

  previousL = l;
  previousR = r;

  previousBL = bl;
  previousBR = br;


  opponentDirection =
    getOpponentDirection();


  classifyOpponentMotion();


  lastTrackingUpdate = now;

}


// ============================================================================
// LINE SENSORS
// ============================================================================

bool frontLeftLine() {

  return digitalRead(
    LINE_FRONT_LEFT_PIN
  ) == LINE_DETECTED;

}


bool frontRightLine() {

  return digitalRead(
    LINE_FRONT_RIGHT_PIN
  ) == LINE_DETECTED;

}


bool backLeftLine() {

  return digitalRead(
    LINE_BACK_LEFT_PIN
  ) == LINE_DETECTED;

}


bool backRightLine() {

  return digitalRead(
    LINE_BACK_RIGHT_PIN
  ) == LINE_DETECTED;

}


// ============================================================================
// START ESCAPE
// ============================================================================

void beginEscape(EscapeState state,
                 unsigned long duration) {

  escapeState = state;

  escapeStarted = millis();

  escapeDuration = duration;

}


// ============================================================================
// BOUNDARY ESCAPE CONTROLLER
// ============================================================================
//
// Completely non-blocking.
//
// The robot continues checking its sensors during escape.
// ============================================================================

bool handleBoundary() {

  bool fl = frontLeftLine();
  bool fr = frontRightLine();

  bool bl = backLeftLine();
  bool br = backRightLine();


  // ----------------------------------------------------------
  // ALREADY ESCAPING
  // ----------------------------------------------------------

  if (escapeState != ESCAPE_NONE) {

    unsigned long elapsed =
      millis() - escapeStarted;


    if (elapsed >= escapeDuration) {

      escapeState = ESCAPE_NONE;

      stopMotors();

      return true;

    }


    switch (escapeState) {

      case ESCAPE_BACK_FROM_FRONT:

        backward(ESCAPE_SPEED);

        break;


      case ESCAPE_TURN_FROM_FRONT_LEFT:

        rotateRight(ESCAPE_SPEED);

        break;


      case ESCAPE_TURN_FROM_FRONT_RIGHT:

        rotateLeft(ESCAPE_SPEED);

        break;


      case ESCAPE_TURN_FROM_FRONT:

        rotateRight(ESCAPE_SPEED);

        break;


      case ESCAPE_FORWARD_FROM_BACK:

        forward(ESCAPE_SPEED);

        break;


      case ESCAPE_TURN_FROM_BACK_LEFT:

        rotateRight(ESCAPE_SPEED);

        break;


      case ESCAPE_TURN_FROM_BACK_RIGHT:

        rotateLeft(ESCAPE_SPEED);

        break;


      case ESCAPE_TURN_FROM_BACK:

        rotateRight(ESCAPE_SPEED);

        break;


      default:

        stopMotors();

        break;

    }


    return true;

  }


  // ----------------------------------------------------------
  // FRONT BOUNDARY
  // ----------------------------------------------------------

  if (fl && fr) {

    Serial.println("BOUNDARY: FRONT BOTH");


    beginEscape(
      ESCAPE_BACK_FROM_FRONT,
      500
    );


    return true;

  }


  if (fl) {

    Serial.println("BOUNDARY: FRONT LEFT");


    beginEscape(
      ESCAPE_BACK_FROM_FRONT,
      450
    );


    return true;

  }


  if (fr) {

    Serial.println("BOUNDARY: FRONT RIGHT");


    beginEscape(
      ESCAPE_BACK_FROM_FRONT,
      450
    );


    return true;

  }


  // ----------------------------------------------------------
  // REAR BOUNDARY
  // ----------------------------------------------------------

  if (bl && br) {

    Serial.println("BOUNDARY: BACK BOTH");


    beginEscape(
      ESCAPE_FORWARD_FROM_BACK,
      500
    );


    return true;

  }


  if (bl) {

    Serial.println("BOUNDARY: BACK LEFT");


    beginEscape(
      ESCAPE_FORWARD_FROM_BACK,
      450
    );


    return true;

  }


  if (br) {

    Serial.println("BOUNDARY: BACK RIGHT");


    beginEscape(
      ESCAPE_FORWARD_FROM_BACK,
      450
    );


    return true;

  }


  return false;

}


// ============================================================================
// FINISH FRONT ESCAPE WITH TURN
// ============================================================================
//
// This function runs after the robot has moved away from the boundary.
//

void continueEscapeStrategy() {

  if (escapeState != ESCAPE_NONE) {

    return;

  }

}


// ============================================================================
// SEARCH
// ============================================================================

void searchOpponent() {

  static unsigned long searchTimer = 0;

  static bool searchDirection = false;


  unsigned long now = millis();


  if (now - searchTimer > 650) {

    searchDirection =
      !searchDirection;

    searchTimer = now;

  }


  if (searchDirection) {

    drive(
      SEARCH_SPEED,
      -SEARCH_SPEED
    );

  }

  else {

    drive(
      -SEARCH_SPEED,
      SEARCH_SPEED
    );

  }

}


// ============================================================================
// FRONT ATTACK
// ============================================================================

void attackOpponent() {

  Serial.println("TACTIC: ATTACK");


  // If opponent is directly in front,
  // attack at maximum power.

  if (opponentDirection == OPP_FRONT) {

    forward(ATTACK_SPEED);

    return;

  }


  if (opponentDirection == OPP_LEFT) {

    drive(
      60,
      ATTACK_SPEED
    );

    return;

  }


  if (opponentDirection == OPP_RIGHT) {

    drive(
      ATTACK_SPEED,
      60
    );

    return;

  }


  forward(ATTACK_SPEED);

}


// ============================================================================
// DEFENSIVE ATTACK
// ============================================================================
//
// If the opponent is approaching slowly,
// position the robot so that it meets the opponent instead of
// allowing the opponent to dictate the direction of the collision.
//

void defensiveAttack() {

  Serial.println("TACTIC: DEFENSIVE ATTACK");


  if (opponentDirection == OPP_LEFT) {

    drive(
      DEFENSE_SPEED / 2,
      DEFENSE_SPEED
    );

  }

  else if (opponentDirection == OPP_RIGHT) {

    drive(
      DEFENSE_SPEED,
      DEFENSE_SPEED / 2
    );

  }

  else {

    forward(DEFENSE_SPEED);

  }

}


// ============================================================================
// FAST APPROACH TRAP
// ============================================================================
//
// If the opponent is coming quickly:
//
// Do not simply reverse.
//
// Rotate toward the detected side so the robot moves into the
// opponent's path.
//
// ============================================================================

void trapFastOpponent() {

  Serial.println("TACTIC: FAST APPROACH INTERCEPT");


  if (opponentDirection == OPP_LEFT) {

    // Move toward the opponent's path.

    drive(
      -INTERCEPT_SPEED,
      INTERCEPT_SPEED
    );

  }


  else if (opponentDirection == OPP_RIGHT) {

    drive(
      INTERCEPT_SPEED,
      -INTERCEPT_SPEED
    );

  }


  else {

    // Direct high-speed approach.
    // Meet the opponent instead of allowing a passive collision.

    forward(INTERCEPT_SPEED);

  }

}


// ============================================================================
// PURSUIT
// ============================================================================

void pursueOpponent() {

  Serial.println("TACTIC: PURSUIT");


  if (opponentDirection == OPP_LEFT) {

    drive(
      TRACK_SPEED / 2,
      ATTACK_SPEED
    );

  }

  else if (opponentDirection == OPP_RIGHT) {

    drive(
      ATTACK_SPEED,
      TRACK_SPEED / 2
    );

  }

  else {

    forward(ATTACK_SPEED);

  }

}


// ============================================================================
// CROSSING OPPONENT
// ============================================================================

void interceptCrossingOpponent() {

  Serial.println("TACTIC: CUT OFF");


  if (opponentDirection == OPP_LEFT) {

    rotateLeft(TURN_SPEED);

  }

  else if (opponentDirection == OPP_RIGHT) {

    rotateRight(TURN_SPEED);

  }

  else {

    forward(ATTACK_SPEED);

  }

}


// ============================================================================
// MAIN PROACTIVE CONTROLLER
// ============================================================================

void proactiveController() {

  // ----------------------------------------------------------
  // UPDATE EVERYTHING
  // ----------------------------------------------------------

  updateUltrasonic();

  updateOpponentTracking();


  // ----------------------------------------------------------
  // BOUNDARY ALWAYS WINS
  // ----------------------------------------------------------

  if (handleBoundary()) {

    return;

  }


  // ----------------------------------------------------------
  // NO OPPONENT
  // ----------------------------------------------------------

  if (!opponentDetected()) {

    searchOpponent();

    return;

  }


  // ----------------------------------------------------------
  // FAST APPROACH
  // ----------------------------------------------------------

  if (opponentMotion ==
      MOTION_APPROACHING_FAST) {

    trapFastOpponent();

    return;

  }


  // ----------------------------------------------------------
  // SLOW APPROACH
  // ----------------------------------------------------------

  if (opponentMotion ==
      MOTION_APPROACHING_SLOW) {

    defensiveAttack();

    return;

  }


  // ----------------------------------------------------------
  // CROSSING
  // ----------------------------------------------------------

  if (opponentMotion ==
      MOTION_CROSSING) {

    interceptCrossingOpponent();

    return;

  }


  // ----------------------------------------------------------
  // MOVING AWAY
  // ----------------------------------------------------------

  if (opponentMotion ==
      MOTION_MOVING_AWAY) {

    pursueOpponent();

    return;

  }


  // ----------------------------------------------------------
  // STATIONARY / CLOSE
  // ----------------------------------------------------------

  if (opponentMotion ==
      MOTION_STATIONARY) {

    attackOpponent();

    return;

  }


  // ----------------------------------------------------------
  // UNKNOWN
  // ----------------------------------------------------------

  if (opponentDirection == OPP_LEFT) {

    rotateLeft(TRACK_SPEED);

  }

  else if (opponentDirection == OPP_RIGHT) {

    rotateRight(TRACK_SPEED);

  }

  else {

    forward(TRACK_SPEED);

  }

}


// ============================================================================
// STARTUP
// ============================================================================

void startCompetition() {

  stopMotors();


  Serial.println();
  Serial.println("==============================");
  Serial.println("BESOMI SUMO ROBOT");
  Serial.println("==============================");
  Serial.println("5 SECOND START DELAY");


  delay(500);


  for (int i = 5; i >= 1; i--) {

    Serial.print("STARTING: ");

    Serial.println(i);

    delay(1000);

  }


  Serial.println("GO!");

  robotStarted = true;

  robotStartTime = millis();

}


// ============================================================================
// SETUP
// ============================================================================

void setup() {

  // ----------------------------------------------------------
  // UNO Q BRIDGE
  // ----------------------------------------------------------

  Bridge.begin();

  // Current UNO Q versions support Serial through the monitor.
  Serial.begin(115200);


  // ----------------------------------------------------------
  // MOTOR PINS
  // ----------------------------------------------------------

  pinMode(M1_DIR, OUTPUT);
  pinMode(M1_PWM, OUTPUT);

  pinMode(M2_DIR, OUTPUT);
  pinMode(M2_PWM, OUTPUT);

  pinMode(M3_DIR, OUTPUT);
  pinMode(M3_PWM, OUTPUT);

  pinMode(M4_DIR, OUTPUT);
  pinMode(M4_PWM, OUTPUT);


  // ----------------------------------------------------------
  // OBSTACLE SENSORS
  // ----------------------------------------------------------

  pinMode(OB_LEFT_PIN, INPUT);
  pinMode(OB_RIGHT_PIN, INPUT);

  pinMode(OB_BACK_RIGHT_PIN, INPUT);
  pinMode(OB_BACK_LEFT_PIN, INPUT);

  pinMode(OB_FRONT_RIGHT_PIN, INPUT);
  pinMode(OB_FRONT_LEFT_PIN, INPUT);


  // ----------------------------------------------------------
  // LINE SENSORS
  // ----------------------------------------------------------

  pinMode(LINE_FRONT_LEFT_PIN, INPUT);
  pinMode(LINE_FRONT_RIGHT_PIN, INPUT);

  pinMode(LINE_BACK_LEFT_PIN, INPUT);
  pinMode(LINE_BACK_RIGHT_PIN, INPUT);


  // ----------------------------------------------------------
  // ULTRASONIC
  // ----------------------------------------------------------

  pinMode(
    ULTRASONIC_PIN,
    INPUT
  );


  // ----------------------------------------------------------
  // ADC
  // ----------------------------------------------------------

  analogReadResolution(14);


  // ----------------------------------------------------------
  // MOTOR INITIALIZATION
  // ----------------------------------------------------------

  stopMotors();


  delay(1000);


  Serial.println("SYSTEM READY");


  // ----------------------------------------------------------
  // COMPETITION START
  // ----------------------------------------------------------

  startCompetition();

}


// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {

  if (!robotStarted) {

    stopMotors();

    return;

  }


  // Everything is continuously evaluated.

  proactiveController();

}