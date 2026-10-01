// ============================================================================
// PROACTIVE SUMO CONTROLLER
// Arduino UNO Q
// ============================================================================


// ASSUMING THE USE OF THIS LIB       https://github.com/pololu/dual-g2-high-power-motor-shield.git

// ============================================================================
// MOTOR PINS
// ============================================================================

const int M1_DIR = D7;
const int M1_PWM = D9;

const int M2_DIR = D8;
const int M2_PWM = D10;

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
// START BUTTON
// ============================================================================

const int START_BUTTON_PIN = A3;


// ============================================================================
// MOTOR DIRECTION
// ============================================================================

bool INVERT_M1 = false;
bool INVERT_M2 = false;

bool INVERT_M3 = true;
bool INVERT_M4 = true;


// ============================================================================
// SPEEDS
// ============================================================================

const int MAX_SPEED = 255;

const int SEARCH_SPEED = 120;

const int TRACK_SPEED = 150;

const int TURN_SPEED = 190;

const int ATTACK_SPEED = 255;

const int DEFENSE_SPEED = 210;

const int ESCAPE_SPEED = 255;

const int TRAP_SPEED = 180;


// ============================================================================
// SENSOR STATES
// ============================================================================

const int SENSOR_DETECTED = LOW;
const int LINE_DETECTED   = LOW;


// ============================================================================
// OPPONENT TRACKING
// ============================================================================

enum OpponentDirection {

  OPP_NONE,
  OPP_LEFT,
  OPP_FRONT,
  OPP_RIGHT,
  OPP_BACK

};


enum OpponentMotion {

  MOTION_UNKNOWN,
  MOTION_APPROACHING,
  MOTION_SLOW,
  MOTION_MOVING_AWAY,
  MOTION_CROSSING

};


OpponentDirection opponentDirection = OPP_NONE;

OpponentMotion opponentMotion = MOTION_UNKNOWN;


// Previous sensor state

bool previousFL = false;
bool previousFR = false;
bool previousL  = false;
bool previousR  = false;

bool previousBL = false;
bool previousBR = false;


// Timing

unsigned long previousTrackingTime = 0;

unsigned long lastOpponentSeen = 0;

unsigned long lastDecisionTime = 0;


// How long the opponent has been visible

unsigned long opponentVisibleSince = 0;


// ============================================================================
// MOTOR FUNCTION
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

  setMotor(M1_DIR, M1_PWM, leftSpeed, INVERT_M1);

  setMotor(M2_DIR, M2_PWM, leftSpeed, INVERT_M2);

  setMotor(M3_DIR, M3_PWM, rightSpeed, INVERT_M3);

  setMotor(M4_DIR, M4_PWM, rightSpeed, INVERT_M4);

}


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
// READ OPPONENT SENSORS
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
// DETERMINE OPPONENT DIRECTION
// ============================================================================

OpponentDirection getOpponentDirection() {

  bool fl = FL();
  bool fr = FR();

  bool l = L();
  bool r = R();

  bool bl = BL();
  bool br = BR();


  // Directly in front

  if (fl && fr) {

    return OPP_FRONT;

  }


  // Front left

  if (fl) {

    return OPP_LEFT;

  }


  // Front right

  if (fr) {

    return OPP_RIGHT;

  }


  // Side left

  if (l) {

    return OPP_LEFT;

  }


  // Side right

  if (r) {

    return OPP_RIGHT;

  }


  // Rear

  if (bl || br) {

    return OPP_BACK;

  }


  return OPP_NONE;

}


// ============================================================================
// OPPONENT MOTION ESTIMATION
// ============================================================================
//
// We cannot measure exact velocity with binary IR sensors.
//
// Instead, we look at how the sensor pattern changes.
//
// Example:
//
// Previous:
//       LEFT
//
// Current:
//       FRONT
//
// This means the opponent moved toward us.
//
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

    if (opponentVisibleSince == 0) {

      opponentVisibleSince = now;

    }

  }
  else {

    opponentVisibleSince = 0;

  }


  // ----------------------------------------------------------
  // APPROACHING
  // ----------------------------------------------------------

  bool previousSide =
    previousFL ||
    previousFR ||
    previousL  ||
    previousR;

  bool currentFront =
    fl || fr;


  if (previousSide && currentFront) {

    opponentMotion = MOTION_APPROACHING;

  }


  // ----------------------------------------------------------
  // FAST APPROACH
  // ----------------------------------------------------------

  bool previousRear =
    previousBL ||
    previousBR;


  if (previousRear && currentFront) {

    opponentMotion = MOTION_APPROACHING;

  }


  // ----------------------------------------------------------
  // CROSSING
  // ----------------------------------------------------------

  if ((previousFL && r) ||
      (previousFR && l) ||
      (previousL && fr) ||
      (previousR && fl)) {

    opponentMotion = MOTION_CROSSING;

  }


  // ----------------------------------------------------------
  // MOVING AWAY
  // ----------------------------------------------------------

  if ((previousFL || previousFR) &&
      (bl || br)) {

    opponentMotion = MOTION_MOVING_AWAY;

  }


  // ----------------------------------------------------------
  // SLOW / STABLE
  // ----------------------------------------------------------

  if (opponentDetected()) {

    if (now - previousTrackingTime > 180) {

      if (fl == previousFL &&
          fr == previousFR &&
          l  == previousL  &&
          r  == previousR) {

        opponentMotion = MOTION_SLOW;

      }

    }

  }


  // Save current state

  previousFL = fl;
  previousFR = fr;

  previousL = l;
  previousR = r;

  previousBL = bl;
  previousBR = br;

  previousTrackingTime = now;


  opponentDirection = getOpponentDirection();

}


// ============================================================================
// LINE DETECTION
// ============================================================================

bool frontLeftLine() {

  return digitalRead(LINE_FRONT_LEFT_PIN) == LINE_DETECTED;

}


bool frontRightLine() {

  return digitalRead(LINE_FRONT_RIGHT_PIN) == LINE_DETECTED;

}


bool backLeftLine() {

  return digitalRead(LINE_BACK_LEFT_PIN) == LINE_DETECTED;

}


bool backRightLine() {

  return digitalRead(LINE_BACK_RIGHT_PIN) == LINE_DETECTED;

}


bool anyLineDetected() {

  return frontLeftLine() ||
         frontRightLine() ||
         backLeftLine() ||
         backRightLine();

}


// ============================================================================
// AGGRESSIVE FRONT ESCAPE
// ============================================================================
//
// The robot moves substantially away from the line before returning
// to opponent tracking.
//
// ============================================================================

void escapeFromFrontLeft() {

  Serial.println("BOUNDARY FRONT LEFT");

  backward(ESCAPE_SPEED);

  delay(450);

  rotateRight(ESCAPE_SPEED);

  delay(450);

}


void escapeFromFrontRight() {

  Serial.println("BOUNDARY FRONT RIGHT");

  backward(ESCAPE_SPEED);

  delay(450);

  rotateLeft(ESCAPE_SPEED);

  delay(450);

}


void escapeFromFront() {

  Serial.println("BOUNDARY FRONT");

  backward(ESCAPE_SPEED);

  delay(550);

  rotateRight(ESCAPE_SPEED);

  delay(500);

}


void escapeFromBackLeft() {

  Serial.println("BOUNDARY BACK LEFT");

  forward(ESCAPE_SPEED);

  delay(450);

  rotateRight(ESCAPE_SPEED);

  delay(450);

}


void escapeFromBackRight() {

  Serial.println("BOUNDARY BACK RIGHT");

  forward(ESCAPE_SPEED);

  delay(450);

  rotateLeft(ESCAPE_SPEED);

  delay(450);

}


void escapeFromBack() {

  Serial.println("BOUNDARY BACK");

  forward(ESCAPE_SPEED);

  delay(550);

  rotateRight(ESCAPE_SPEED);

  delay(500);

}


// ============================================================================
// BOUNDARY CONTROLLER
// ============================================================================

bool handleBoundary() {

  bool fl = frontLeftLine();
  bool fr = frontRightLine();

  bool bl = backLeftLine();
  bool br = backRightLine();


  // Front gets highest priority

  if (fl && fr) {

    escapeFromFront();

    return true;

  }


  if (fl) {

    escapeFromFrontLeft();

    return true;

  }


  if (fr) {

    escapeFromFrontRight();

    return true;

  }


  // Rear

  if (bl && br) {

    escapeFromBack();

    return true;

  }


  if (bl) {

    escapeFromBackLeft();

    return true;

  }


  if (br) {

    escapeFromBackRight();

    return true;

  }


  return false;

}


// ============================================================================
// ATTACK
// ============================================================================

void attackFront() {

  Serial.println("OFFENSE: ATTACK");

  forward(ATTACK_SPEED);

}


// ============================================================================
// INTERCEPT
// ============================================================================
//
// If the opponent is approaching rapidly, do not simply chase it.
//
// Turn toward its path and intercept it.
//
// ============================================================================

void interceptOpponent() {

  Serial.println("TACTIC: INTERCEPT");


  if (opponentDirection == OPP_LEFT) {

    drive(
      -TRAP_SPEED,
      TRAP_SPEED
    );

  }

  else if (opponentDirection == OPP_RIGHT) {

    drive(
      TRAP_SPEED,
      -TRAP_SPEED
    );

  }

  else {

    forward(TRAP_SPEED);

  }

}


// ============================================================================
// DEFENSE
// ============================================================================

void defensiveResponse() {

  Serial.println("TACTIC: DEFENSE");


  // Keep the robot moving.
  // Do not sit still and allow a straight push.

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
// SEARCH
// ============================================================================

void searchOpponent() {

  static unsigned long searchTimer = 0;

  static bool direction = false;


  if (millis() - searchTimer > 700) {

    direction = !direction;

    searchTimer = millis();

  }


  if (direction) {

    rotateRight(SEARCH_SPEED);

  }

  else {

    rotateLeft(SEARCH_SPEED);

  }

}


// ============================================================================
// MAIN PROACTIVE DECISION
// ============================================================================

void proactiveController() {

  updateOpponentTracking();


  // ==========================================================
  // PRIORITY 1
  // BOUNDARY
  // ==========================================================

  if (handleBoundary()) {

    return;

  }


  // ==========================================================
  // NO OPPONENT
  // SEARCH
  // ==========================================================

  if (!opponentDetected()) {

    searchOpponent();

    return;

  }


  // ==========================================================
  // OPPONENT APPROACHING QUICKLY
  // INTERCEPT / TRAP
  // ==========================================================

  if (opponentMotion == MOTION_APPROACHING) {

    interceptOpponent();

    return;

  }


  // ==========================================================
  // OPPONENT CROSSING
  // TURN INTO ITS PATH
  // ==========================================================

  if (opponentMotion == MOTION_CROSSING) {

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

    return;

  }


  // ==========================================================
  // OPPONENT SLOW OR STATIONARY
  // ATTACK
  // ==========================================================

  if (opponentMotion == MOTION_SLOW) {

    if (opponentDirection == OPP_FRONT) {

      attackFront();

    }

    else if (opponentDirection == OPP_LEFT) {

      rotateLeft(TURN_SPEED);

    }

    else if (opponentDirection == OPP_RIGHT) {

      rotateRight(TURN_SPEED);

    }

    else {

      forward(ATTACK_SPEED);

    }

    return;

  }


  // ==========================================================
  // OPPONENT MOVING AWAY
  // PURSUE
  // ==========================================================

  if (opponentMotion == MOTION_MOVING_AWAY) {

    Serial.println("TACTIC: PURSUIT");

    forward(ATTACK_SPEED);

    return;

  }


  // ==========================================================
  // UNKNOWN
  // DEFAULT TO AGGRESSIVE TRACKING
  // ==========================================================

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
// START
// ============================================================================

void waitForStart() {

  stopMotors();

  Serial.println("================================");
  Serial.println("SUMO ROBOT READY");
  Serial.println("Press START");
  Serial.println("================================");


  while (digitalRead(START_BUTTON_PIN) == LOW) {

    delay(10);

  }


  while (digitalRead(START_BUTTON_PIN) == HIGH) {

    delay(10);

  }


  Serial.println("START");


  for (int i = 5; i >= 1; i--) {

    Serial.print("Starting in ");
    Serial.println(i);

    delay(1000);

  }


  Serial.println("GO!");

  robotStarted = true;

}


// ============================================================================
// SETUP
// ============================================================================

bool robotStarted = false;


void setup() {

  Serial.begin(115200);

  delay(1000);


  // Motors

  pinMode(M1_DIR, OUTPUT);
  pinMode(M1_PWM, OUTPUT);

  pinMode(M2_DIR, OUTPUT);
  pinMode(M2_PWM, OUTPUT);

  pinMode(M3_DIR, OUTPUT);
  pinMode(M3_PWM, OUTPUT);


  pinMode(M4_DIR, OUTPUT);
  pinMode(M4_PWM, OUTPUT);


  // Obstacles

  pinMode(OB_LEFT_PIN, INPUT);
  pinMode(OB_RIGHT_PIN, INPUT);

  pinMode(OB_BACK_RIGHT_PIN, INPUT);
  pinMode(OB_BACK_LEFT_PIN, INPUT);

  pinMode(OB_FRONT_RIGHT_PIN, INPUT);
  pinMode(OB_FRONT_LEFT_PIN, INPUT);


  // Lines

  pinMode(LINE_FRONT_LEFT_PIN, INPUT);
  pinMode(LINE_FRONT_RIGHT_PIN, INPUT);

  pinMode(LINE_BACK_LEFT_PIN, INPUT);
  pinMode(LINE_BACK_RIGHT_PIN, INPUT);


  // Start

  pinMode(START_BUTTON_PIN, INPUT_PULLUP);


  stopMotors();

  delay(500);

  waitForStart();

}


// ============================================================================
// LOOP
// ============================================================================

void loop() {

  if (!robotStarted) {

    waitForStart();

    return;

  }


  proactiveController();

  delay(5);

}