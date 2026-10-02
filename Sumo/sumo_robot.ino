// ============================================================================
// BESOMI ACADEMY 2026
// PROACTIVE SUMO ROBOT CONTROLLER
// Arduino UNO Q
//
// ============================================================================
// SENSOR CONFIGURATION
// ============================================================================
//
// IR SENSORS: 5
//
// D2 = FRONT LEFT IR
// D0 = FRONT RIGHT IR
// D1 = LEFT IR
// D3 = RIGHT IR           needs to be flipped
// D4 = BACK CENTER IR
//
// LINE SENSORS: 3
//
// A0 = FRONT LEFT LINE
// A1 = FRONT RIGHT LINE
// A4 = BACK CENTER LINE
//
// ULTRASONIC:
//
// A2 = TRIG
// A3 = ECHO
//
// START BUTTON:
//
// D11 = START BUTTON
//
// ============================================================================


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
// MOTOR INVERSION
// ============================================================================

const bool INVERT_M1 = false;
const bool INVERT_M2 = false;

const bool INVERT_M3 = true;
const bool INVERT_M4 = true;


// ============================================================================
// IR SENSOR PINS
// ============================================================================

const int IR_FRONT_LEFT_PIN  = D2;
const int IR_FRONT_RIGHT_PIN = D0;

const int IR_LEFT_PIN  = D1;
const int IR_RIGHT_PIN = D3;

const int IR_BACK_PIN = D4;


// ============================================================================
// LINE SENSOR PINS
// ============================================================================

const int LINE_FRONT_LEFT_PIN  = A0;
const int LINE_FRONT_RIGHT_PIN = A1;

const int LINE_BACK_PIN = A4;


// ============================================================================
// START BUTTON
// ============================================================================

const int START_BUTTON_PIN = D11;


// ============================================================================
// ULTRASONIC
// ============================================================================

const int ULTRASONIC_TRIG_PIN = A2;
const int ULTRASONIC_ECHO_PIN = A3;


// ============================================================================
// SENSOR LOGIC
// ============================================================================
//
// MZ80-style NPN outputs normally need a pull-up.
// All sensor pins therefore use INPUT_PULLUP.
//
// LOW = DETECTED
// HIGH = NOT DETECTED
//

const int SENSOR_DETECTED = LOW;
const int LINE_DETECTED   = LOW;


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
// OPENING MOVE
// ============================================================================

const unsigned long OPENING_MOVE_DURATION = 250;


// ============================================================================
// ULTRASONIC SETTINGS
// ============================================================================

const float SOUND_SPEED_CM_PER_US = 0.0343;

const float MIN_TARGET_DISTANCE = 3.0;

const float MAX_TARGET_DISTANCE = 250.0;

const unsigned long ULTRASONIC_INTERVAL = 30;

const unsigned long ULTRASONIC_TIMEOUT = 8000;


// ============================================================================
// MOTION DETECTION SETTINGS
// ============================================================================

const float FAST_CLOSING_SPEED = 80.0;

const float SLOW_CLOSING_SPEED = 15.0;

const float MOVING_AWAY_SPEED = -20.0;

const unsigned long IR_SLOW_THRESHOLD = 180;


// ============================================================================
// LAST-SEEN SETTINGS
// ============================================================================

const unsigned long LAST_SEEN_MEMORY = 500;

const unsigned long LAST_SEEN_TURN_TIME = 250;


// ============================================================================
// ULTRASONIC VARIABLES
// ============================================================================

float ultrasonicDistance = -1.0;

float previousDistance = -1.0;

float closingSpeed = 0.0;

float previousClosingSpeed = 0.0;

float closingAcceleration = 0.0;

unsigned long previousDistanceTime = 0;

unsigned long lastUltrasonicRead = 0;

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

OpponentDirection lastOpponentDirection = OPP_NONE;


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
// PREVIOUS IR STATES
// ============================================================================

bool previousFL = false;
bool previousFR = false;

bool previousL = false;
bool previousR = false;

bool previousBL = false;


// ============================================================================
// TRACKING TIMERS
// ============================================================================

unsigned long lastOpponentSeen = 0;

unsigned long lastPatternChangeTime = 0;

unsigned long lastTrackingUpdate = 0;


// ============================================================================
// BOUNDARY ESCAPE
// ============================================================================

enum EscapeState {

  ESCAPE_NONE,

  ESCAPE_BACK_FROM_FRONT,

  ESCAPE_TURN_FROM_FRONT,

  ESCAPE_FORWARD_FROM_BACK,

  ESCAPE_TURN_FROM_BACK

};


EscapeState escapeState = ESCAPE_NONE;

unsigned long escapeStarted = 0;

unsigned long escapeDuration = 0;


// false = right
// true  = left

bool escapeTurnLeft = false;


// ============================================================================
// ROBOT STATE
// ============================================================================

bool robotStarted = false;

bool openingMoveActive = false;

unsigned long openingMoveStarted = 0;


// ============================================================================
// SEARCH STATE
// ============================================================================

bool searchDirectionLeft = false;

unsigned long searchDirectionChanged = 0;

unsigned long lastSeenTurnStarted = 0;

bool lastSeenTurnActive = false;


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

    analogWrite(
      pwmPin,
      speed
    );

  }

  else if (speed < 0) {

    digitalWrite(dirPin, LOW);

    analogWrite(
      pwmPin,
      -speed
    );

  }

  else {

    analogWrite(
      pwmPin,
      0
    );

  }

}


// ============================================================================
// DRIVE
// ============================================================================

void drive(
  int leftSpeed,
  int rightSpeed
) {

  leftSpeed = constrain(
    leftSpeed,
    -255,
    255
  );

  rightSpeed = constrain(
    rightSpeed,
    -255,
    255
  );


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

  drive(
    0,
    0
  );

}


void forward(
  int speed
) {

  drive(
    speed,
    speed
  );

}


void backward(
  int speed
) {

  drive(
    -speed,
    -speed
  );

}


void rotateLeft(
  int speed
) {

  drive(
    -speed,
    speed
  );

}


void rotateRight(
  int speed
) {

  drive(
    speed,
    -speed
  );

}


// ============================================================================
// IR READ FUNCTIONS
// ============================================================================

bool FL() {

  return digitalRead(
    IR_FRONT_LEFT_PIN
  ) == SENSOR_DETECTED;

}


bool FR() {

  return digitalRead(
    IR_FRONT_RIGHT_PIN
  ) == SENSOR_DETECTED;

}


bool L() {

  return digitalRead(
    IR_LEFT_PIN
  ) == SENSOR_DETECTED;

}


bool R() {

  return digitalRead(
    IR_RIGHT_PIN
  ) == SENSOR_DETECTED;

}


bool BL() {

  return digitalRead(
    IR_BACK_PIN
  ) == SENSOR_DETECTED;

}


// ============================================================================
// LINE READ FUNCTIONS
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


bool backLine() {

  return digitalRead(
    LINE_BACK_PIN
  ) == LINE_DETECTED;

}


// ============================================================================
// ULTRASONIC READ
// ============================================================================

float readUltrasonic() {

  digitalWrite(
    ULTRASONIC_TRIG_PIN,
    LOW
  );

  delayMicroseconds(2);

  digitalWrite(
    ULTRASONIC_TRIG_PIN,
    HIGH
  );

  delayMicroseconds(10);

  digitalWrite(
    ULTRASONIC_TRIG_PIN,
    LOW
  );


  unsigned long duration =
    pulseIn(
      ULTRASONIC_ECHO_PIN,
      HIGH,
      ULTRASONIC_TIMEOUT
    );


  if (duration == 0) {

    return -1.0;

  }


  float distance =
    (
      duration *
      SOUND_SPEED_CM_PER_US
    ) / 2.0;


  if (
    distance < MIN_TARGET_DISTANCE ||
    distance > MAX_TARGET_DISTANCE
  ) {

    return -1.0;

  }


  return distance;

}


// ============================================================================
// ULTRASONIC UPDATE
// ============================================================================
//
// Consecutive readings are used to calculate closing speed.
//
// Positive closing speed = opponent getting closer.
// Negative closing speed = opponent moving away.
//

void updateUltrasonic() {

  unsigned long now =
    millis();


  if (
    now - lastUltrasonicRead <
    ULTRASONIC_INTERVAL
  ) {

    return;

  }


  lastUltrasonicRead =
    now;


  float newDistance =
    readUltrasonic();


  if (newDistance < 0) {

    ultrasonicValid =
      false;

    return;

  }


  ultrasonicValid =
    true;


  if (previousDistance > 0) {

    unsigned long dt =
      now - previousDistanceTime;


    if (dt > 0) {

      float instantClosingSpeed =
        (
          previousDistance -
          newDistance
        )
        /
        (
          dt / 1000.0
        );


      closingSpeed =
        (
          closingSpeed *
          0.65
        )
        +
        (
          instantClosingSpeed *
          0.35
        );


      float instantAcceleration =
        (
          closingSpeed -
          previousClosingSpeed
        )
        /
        (
          dt / 1000.0
        );


      closingAcceleration =
        (
          closingAcceleration *
          0.70
        )
        +
        (
          instantAcceleration *
          0.30
        );


      previousClosingSpeed =
        closingSpeed;

    }

  }


  previousDistance =
    newDistance;

  previousDistanceTime =
    now;

  ultrasonicDistance =
    newDistance;

}


// ============================================================================
// OPPONENT DETECTION
// ============================================================================

bool opponentDetected() {

  if (
    FL() ||
    FR() ||
    L() ||
    R() ||
    BL()
  ) {

    return true;

  }


  if (
    ultrasonicValid &&
    ultrasonicDistance >= MIN_TARGET_DISTANCE &&
    ultrasonicDistance <= MAX_TARGET_DISTANCE
  ) {

    return true;

  }


  return false;

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


  // Both front IR sensors

  if (
    fl &&
    fr
  ) {

    return OPP_FRONT;

  }


  // Front center ultrasonic

  if (
    ultrasonicValid &&
    ultrasonicDistance >= MIN_TARGET_DISTANCE &&
    ultrasonicDistance <= MAX_TARGET_DISTANCE
  ) {

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


  if (bl) {

    return OPP_BACK;

  }


  return OPP_NONE;

}


// ============================================================================
// IR MOTION CLASSIFICATION
// ============================================================================
//
// This is the important timestamp fix.
//
// lastPatternChangeTime changes ONLY when the IR pattern changes.
//
// It is not updated every tracking cycle.
//

OpponentMotion classifyIRMotion(
  unsigned long now
) {

  bool fl = FL();
  bool fr = FR();

  bool l = L();
  bool r = R();

  bool bl = BL();


  bool patternChanged =
    (fl != previousFL) ||
    (fr != previousFR) ||
    (l  != previousL) ||
    (r  != previousR) ||
    (bl != previousBL);


  if (patternChanged) {

    lastPatternChangeTime =
      now;

  }


  bool currentFront =
    fl ||
    fr;


  bool previousFront =
    previousFL ||
    previousFR;


  bool currentSide =
    l ||
    r;


  bool previousSide =
    previousL ||
    previousR;


  bool currentBack =
    bl;


  bool previousBack =
    previousBL;


  // ----------------------------------------------------------
  // REAR TO FRONT
  // ----------------------------------------------------------

  if (
    previousBack &&
    currentFront
  ) {

    return MOTION_APPROACHING_FAST;

  }


  // ----------------------------------------------------------
  // SIDE TO FRONT
  // ----------------------------------------------------------

  if (
    previousSide &&
    currentFront
  ) {

    return MOTION_APPROACHING_SLOW;

  }


  // ----------------------------------------------------------
  // SIDE CROSSING
  // ----------------------------------------------------------

  if (
    (previousFL && r) ||
    (previousFR && l) ||
    (previousL && fr) ||
    (previousR && fl)
  ) {

    return MOTION_CROSSING;

  }


  // ----------------------------------------------------------
  // FRONT PATTERN CHANGED
  // ----------------------------------------------------------

  if (
    currentFront &&
    !previousFront
  ) {

    return MOTION_APPROACHING_SLOW;

  }


  // ----------------------------------------------------------
  // PATTERN HAS BEEN STABLE
  // ----------------------------------------------------------

  if (
    opponentDetected()
  ) {

    unsigned long stableTime =
      now - lastPatternChangeTime;


    if (
      stableTime >=
      IR_SLOW_THRESHOLD
    ) {

      return MOTION_STATIONARY;

    }


    return MOTION_APPROACHING_SLOW;

  }


  return MOTION_UNKNOWN;

}


// ============================================================================
// OPPONENT MOTION UPDATE
// ============================================================================

void classifyOpponentMotion() {

  unsigned long now =
    millis();


  // ----------------------------------------------------------
  // ULTRASONIC HAS PRIORITY WHEN VALID
  // ----------------------------------------------------------

  if (
    ultrasonicValid
  ) {

    if (
      closingSpeed >=
      FAST_CLOSING_SPEED
    ) {

      opponentMotion =
        MOTION_APPROACHING_FAST;

      return;

    }


    if (
      closingSpeed >=
      SLOW_CLOSING_SPEED
    ) {

      opponentMotion =
        MOTION_APPROACHING_SLOW;

      return;

    }


    if (
      closingSpeed <=
      MOVING_AWAY_SPEED
    ) {

      opponentMotion =
        MOTION_MOVING_AWAY;

      return;

    }


    // If the ultrasonic distance is almost constant,
    // fall back to IR information.

  }


  // ----------------------------------------------------------
  // IR FALLBACK
  // ----------------------------------------------------------

  opponentMotion =
    classifyIRMotion(
      now
    );

}


// ============================================================================
// OPPONENT TRACKING UPDATE
// ============================================================================

void updateOpponentTracking() {

  unsigned long now =
    millis();


  // ----------------------------------------------------------
  // SAVE CURRENT SENSOR STATE
  // ----------------------------------------------------------

  bool currentFL = FL();
  bool currentFR = FR();

  bool currentL = L();
  bool currentR = R();

  bool currentBL = BL();


  // ----------------------------------------------------------
  // DETECT PATTERN CHANGE
  // ----------------------------------------------------------

  bool patternChanged =
    (currentFL != previousFL) ||
    (currentFR != previousFR) ||
    (currentL != previousL) ||
    (currentR != previousR) ||
    (currentBL != previousBL);


  if (patternChanged) {

    lastPatternChangeTime =
      now;

  }


  // ----------------------------------------------------------
  // DETECT OPPONENT
  // ----------------------------------------------------------

  bool detected =
    opponentDetected();


  if (detected) {

    lastOpponentSeen =
      now;


    OpponentDirection currentDirection =
      getOpponentDirection();


    if (
      currentDirection !=
      OPP_NONE
    ) {

      opponentDirection =
        currentDirection;


      lastOpponentDirection =
        currentDirection;

    }

  }

  else {

    // Do NOT erase the last direction immediately.
    // It is used by the last-seen search behavior.

    opponentDirection =
      OPP_NONE;

  }


  // ----------------------------------------------------------
  // MOTION
  // ----------------------------------------------------------

  if (detected) {

    classifyOpponentMotion();

  }

  else {

    opponentMotion =
      MOTION_UNKNOWN;

  }


  // ----------------------------------------------------------
  // SAVE CURRENT STATES FOR NEXT CYCLE
  // ----------------------------------------------------------

  previousFL =
    currentFL;

  previousFR =
    currentFR;

  previousL =
    currentL;

  previousR =
    currentR;

  previousBL =
    currentBL;


  lastTrackingUpdate =
    now;

}


// ============================================================================
// START ESCAPE
// ============================================================================

void startFrontEscape(
  unsigned long duration,
  bool turnLeft
) {

  escapeState =
    ESCAPE_BACK_FROM_FRONT;

  escapeStarted =
    millis();

  escapeDuration =
    duration;

  escapeTurnLeft =
    turnLeft;

}


void startRearEscape(
  unsigned long duration,
  bool turnLeft
) {

  escapeState =
    ESCAPE_FORWARD_FROM_BACK;

  escapeStarted =
    millis();

  escapeDuration =
    duration;

  escapeTurnLeft =
    turnLeft;

}


// ============================================================================
// BOUNDARY ESCAPE
// ============================================================================
//
// Entirely non-blocking.
//
// No delay() is used here.
//
// FRONT LEFT:
//     Back up -> turn RIGHT
//
// FRONT RIGHT:
//     Back up -> turn LEFT
//
// BOTH FRONT:
//     Back up -> turn LEFT
//
// REAR CENTER:
//     Move forward -> turn RIGHT
//

bool handleBoundary() {

  bool fl =
    frontLeftLine();

  bool fr =
    frontRightLine();

  bool back =
    backLine();


  // ========================================================================
  // ALREADY ESCAPING
  // ========================================================================

  if (
    escapeState !=
    ESCAPE_NONE
  ) {

    unsigned long elapsed =
      millis() - escapeStarted;


    // ----------------------------------------------------------------------
    // WHILE BACKING FROM FRONT, WATCH REAR
    // ----------------------------------------------------------------------

    if (
      escapeState ==
      ESCAPE_BACK_FROM_FRONT
    ) {

      if (back) {

        startRearEscape(
          250,
          false
        );

        return true;

      }

    }


    // ----------------------------------------------------------------------
    // WHILE MOVING FROM REAR, WATCH FRONT
    // ----------------------------------------------------------------------

    if (
      escapeState ==
      ESCAPE_FORWARD_FROM_BACK
    ) {

      if (
        fl ||
        fr
      ) {

        if (
          fl &&
          !fr
        ) {

          startFrontEscape(
            250,
            false
          );

        }

        else if (
          fr &&
          !fl
        ) {

          startFrontEscape(
            250,
            true
          );

        }

        else {

          startFrontEscape(
            300,
            true
          );

        }


        return true;

      }

    }


    // ----------------------------------------------------------------------
    // CURRENT PHASE FINISHED
    // ----------------------------------------------------------------------

    if (
      elapsed >=
      escapeDuration
    ) {

      // ----------------------------------------------------
      // BACKUP FINISHED
      // ----------------------------------------------------

      if (
        escapeState ==
        ESCAPE_BACK_FROM_FRONT
      ) {

        escapeState =
          ESCAPE_TURN_FROM_FRONT;

        escapeStarted =
          millis();

        escapeDuration =
          300;

        return true;

      }


      // ----------------------------------------------------
      // FORWARD ESCAPE FINISHED
      // ----------------------------------------------------

      if (
        escapeState ==
        ESCAPE_FORWARD_FROM_BACK
      ) {

        escapeState =
          ESCAPE_TURN_FROM_BACK;

        escapeStarted =
          millis();

        escapeDuration =
          300;

        return true;

      }


      // ----------------------------------------------------
      // TURN FINISHED
      // ----------------------------------------------------

      escapeState =
        ESCAPE_NONE;

      stopMotors();

      return true;

    }


    // ----------------------------------------------------------------------
    // EXECUTE CURRENT PHASE
    // ----------------------------------------------------------------------

    switch (
      escapeState
    ) {

      case ESCAPE_BACK_FROM_FRONT:

        backward(
          ESCAPE_SPEED
        );

        break;


      case ESCAPE_TURN_FROM_FRONT:

        if (
          escapeTurnLeft
        ) {

          rotateLeft(
            ESCAPE_SPEED
          );

        }

        else {

          rotateRight(
            ESCAPE_SPEED
          );

        }

        break;


      case ESCAPE_FORWARD_FROM_BACK:

        forward(
          ESCAPE_SPEED
        );

        break;


      case ESCAPE_TURN_FROM_BACK:

        if (
          escapeTurnLeft
        ) {

          rotateLeft(
            ESCAPE_SPEED
          );

        }

        else {

          rotateRight(
            ESCAPE_SPEED
          );

        }

        break;


      default:

        stopMotors();

        break;

    }


    return true;

  }


  // ========================================================================
  // NEW FRONT ESCAPE
  // ========================================================================

  // BOTH FRONT SENSORS

  if (
    fl &&
    fr
  ) {

    startFrontEscape(
      400,
      true
    );

    return true;

  }


  // FRONT LEFT

  if (fl) {

    // Left side of robot reached edge.
    // Turn right.

    startFrontEscape(
      300,
      false
    );

    return true;

  }


  // FRONT RIGHT

  if (fr) {

    // Right side reached edge.
    // Turn left.

    startFrontEscape(
      300,
      true
    );

    return true;

  }


  // ========================================================================
  // NEW REAR ESCAPE
  // ========================================================================

  if (back) {

    // Only one rear sensor exists, so no left/right
    // information is available.
    //
    // Use right turn as the default.

    startRearEscape(
      300,
      false
    );

    return true;

  }


  return false;

}


// ============================================================================
// OPENING MOVE
// ============================================================================

void openingMove() {

  unsigned long elapsed =
    millis() - openingMoveStarted;


  if (
    elapsed <
    OPENING_MOVE_DURATION
  ) {

    forward(
      ATTACK_SPEED
    );

    return;

  }


  openingMoveActive =
    false;

}


// ============================================================================
// LAST-SEEN PURSUIT
// ============================================================================
//
// When the opponent disappears, do not immediately begin a random sweep.
//
// First turn toward the last known direction.
//

bool handleLastSeen() {

  if (
    lastOpponentSeen == 0
  ) {

    return false;

  }


  unsigned long now =
    millis();


  unsigned long timeSinceSeen =
    now - lastOpponentSeen;


  if (
    timeSinceSeen >
    LAST_SEEN_MEMORY
  ) {

    lastSeenTurnActive =
      false;

    return false;

  }


  // --------------------------------------------------------------------------
  // FRONT
  // --------------------------------------------------------------------------

  if (
    lastOpponentDirection ==
    OPP_FRONT
  ) {

    forward(
      TRACK_SPEED
    );

    return true;

  }


  // --------------------------------------------------------------------------
  // LEFT
  // --------------------------------------------------------------------------

  if (
    lastOpponentDirection ==
    OPP_LEFT
  ) {

    if (!lastSeenTurnActive) {

      lastSeenTurnActive =
        true;

      lastSeenTurnStarted =
        now;

    }


    if (
      now - lastSeenTurnStarted <
      LAST_SEEN_TURN_TIME
    ) {

      rotateLeft(
        TURN_SPEED
      );

      return true;

    }

  }


  // --------------------------------------------------------------------------
  // RIGHT
  // --------------------------------------------------------------------------

  if (
    lastOpponentDirection ==
    OPP_RIGHT
  ) {

    if (!lastSeenTurnActive) {

      lastSeenTurnActive =
        true;

      lastSeenTurnStarted =
        now;

    }


    if (
      now - lastSeenTurnStarted <
      LAST_SEEN_TURN_TIME
    ) {

      rotateRight(
        TURN_SPEED
      );

      return true;

    }

  }


  // --------------------------------------------------------------------------
  // BACK
  // --------------------------------------------------------------------------

  if (
    lastOpponentDirection ==
    OPP_BACK
  ) {

    if (!lastSeenTurnActive) {

      lastSeenTurnActive =
        true;

      lastSeenTurnStarted =
        now;

    }


    if (
      now - lastSeenTurnStarted <
      LAST_SEEN_TURN_TIME
    ) {

      rotateLeft(
        TURN_SPEED
      );

      return true;

    }

  }


  lastSeenTurnActive =
    false;


  return false;

}


// ============================================================================
// SEARCH
// ============================================================================

void searchOpponent() {

  unsigned long now =
    millis();


  // --------------------------------------------------------------------------
  // FIRST USE LAST-SEEN DIRECTION
  // --------------------------------------------------------------------------

  if (
    handleLastSeen()
  ) {

    return;

  }


  // --------------------------------------------------------------------------
  // NORMAL SEARCH SWEEP
  // --------------------------------------------------------------------------

  if (
    now - searchDirectionChanged >
    650
  ) {

    searchDirectionLeft =
      !searchDirectionLeft;

    searchDirectionChanged =
      now;

  }


  if (
    searchDirectionLeft
  ) {

    rotateLeft(
      SEARCH_SPEED
    );

  }

  else {

    rotateRight(
      SEARCH_SPEED
    );

  }

}


// ============================================================================
// ATTACK
// ============================================================================

void attackOpponent() {

  switch (
    opponentDirection
  ) {

    case OPP_LEFT:

      drive(
        60,
        ATTACK_SPEED
      );

      break;


    case OPP_RIGHT:

      drive(
        ATTACK_SPEED,
        60
      );

      break;


    case OPP_FRONT:

      forward(
        ATTACK_SPEED
      );

      break;


    case OPP_BACK:

      rotateLeft(
        TURN_SPEED
      );

      break;


    default:

      forward(
        ATTACK_SPEED
      );

      break;

  }

}


// ============================================================================
// DEFENSIVE ATTACK
// ============================================================================

void defensiveAttack() {

  switch (
    opponentDirection
  ) {

    case OPP_LEFT:

      drive(
        DEFENSE_SPEED / 2,
        DEFENSE_SPEED
      );

      break;


    case OPP_RIGHT:

      drive(
        DEFENSE_SPEED,
        DEFENSE_SPEED / 2
      );

      break;


    case OPP_FRONT:

      forward(
        DEFENSE_SPEED
      );

      break;


    default:

      forward(
        DEFENSE_SPEED
      );

      break;

  }

}


// ============================================================================
// FAST APPROACH
// ============================================================================

void trapFastOpponent() {

  switch (
    opponentDirection
  ) {

    case OPP_LEFT:

      drive(
        -INTERCEPT_SPEED,
        INTERCEPT_SPEED
      );

      break;


    case OPP_RIGHT:

      drive(
        INTERCEPT_SPEED,
        -INTERCEPT_SPEED
      );

      break;


    case OPP_FRONT:

      forward(
        INTERCEPT_SPEED
      );

      break;


    default:

      forward(
        INTERCEPT_SPEED
      );

      break;

  }

}


// ============================================================================
// PURSUIT
// ============================================================================

void pursueOpponent() {

  switch (
    opponentDirection
  ) {

    case OPP_LEFT:

      drive(
        TRACK_SPEED / 2,
        ATTACK_SPEED
      );

      break;


    case OPP_RIGHT:

      drive(
        ATTACK_SPEED,
        TRACK_SPEED / 2
      );

      break;


    case OPP_FRONT:

      forward(
        ATTACK_SPEED
      );

      break;


    default:

      forward(
        TRACK_SPEED
      );

      break;

  }

}


// ============================================================================
// CROSSING INTERCEPT
// ============================================================================

void interceptCrossingOpponent() {

  if (
    opponentDirection ==
    OPP_LEFT
  ) {

    rotateLeft(
      TURN_SPEED
    );

  }

  else if (
    opponentDirection ==
    OPP_RIGHT
  ) {

    rotateRight(
      TURN_SPEED
    );

  }

  else {

    forward(
      ATTACK_SPEED
    );

  }

}


// ============================================================================
// MAIN CONTROLLER
// ============================================================================

void proactiveController() {

  // --------------------------------------------------------------------------
  // SENSOR UPDATES
  // --------------------------------------------------------------------------

  updateUltrasonic();

  updateOpponentTracking();


  // --------------------------------------------------------------------------
  // BOUNDARY HAS ABSOLUTE PRIORITY
  // --------------------------------------------------------------------------

  if (
    handleBoundary()
  ) {

    return;

  }


  // --------------------------------------------------------------------------
  // OPENING MOVE
  // --------------------------------------------------------------------------

  if (
    openingMoveActive
  ) {

    openingMove();

    return;

  }


  // --------------------------------------------------------------------------
  // NO OPPONENT
  // --------------------------------------------------------------------------

  if (
    !opponentDetected()
  ) {

    searchOpponent();

    return;

  }


  // --------------------------------------------------------------------------
  // OPPONENT DETECTED
  // --------------------------------------------------------------------------

  switch (
    opponentMotion
  ) {

    case MOTION_APPROACHING_FAST:

      trapFastOpponent();

      break;


    case MOTION_APPROACHING_SLOW:

      defensiveAttack();

      break;


    case MOTION_CROSSING:

      interceptCrossingOpponent();

      break;


    case MOTION_MOVING_AWAY:

      pursueOpponent();

      break;


    case MOTION_STATIONARY:

      attackOpponent();

      break;


    default:

      attackOpponent();

      break;

  }

}


// ============================================================================
// START BUTTON
// ============================================================================
//
// D11 connected to button.
// Other side of button connected to GND.
//
// INPUT_PULLUP means:
//
// HIGH = released
// LOW  = pressed
//

void waitForStart() {

  stopMotors();


  Serial.println(
    "=============================="
  );

  Serial.println(
    "BESOMI SUMO ROBOT READY"
  );

  Serial.println(
    "PRESS START BUTTON"
  );

  Serial.println(
    "=============================="
  );


  // --------------------------------------------------------------------------
  // WAIT FOR BUTTON PRESS
  // --------------------------------------------------------------------------

  while (
    digitalRead(
      START_BUTTON_PIN
    ) == HIGH
  ) {

    stopMotors();

    delay(5);

  }


  // --------------------------------------------------------------------------
  // DEBOUNCE
  // --------------------------------------------------------------------------

  delay(50);


  // --------------------------------------------------------------------------
  // WAIT FOR RELEASE
  // --------------------------------------------------------------------------

  while (
    digitalRead(
      START_BUTTON_PIN
    ) == LOW
  ) {

    stopMotors();

    delay(5);

  }


  // --------------------------------------------------------------------------
  // COUNTDOWN
  // --------------------------------------------------------------------------

  Serial.println(
    "5"
  );

  delay(1000);


  Serial.println(
    "4"
  );

  delay(1000);


  Serial.println(
    "3"
  );

  delay(1000);


  Serial.println(
    "2"
  );

  delay(1000);


  Serial.println(
    "1"
  );

  delay(1000);


  Serial.println(
    "GO!"
  );


  // --------------------------------------------------------------------------
  // START ROBOT
  // --------------------------------------------------------------------------

  robotStarted =
    true;


  openingMoveActive =
    true;


  openingMoveStarted =
    millis();


  searchDirectionChanged =
    millis();

}


// ============================================================================
// SETUP
// ============================================================================

void setup() {

  Serial.begin(
    115200
  );


  // ==========================================================================
  // MOTOR PINS
  // ==========================================================================

  pinMode(
    M1_DIR,
    OUTPUT
  );

  pinMode(
    M1_PWM,
    OUTPUT
  );


  pinMode(
    M2_DIR,
    OUTPUT
  );

  pinMode(
    M2_PWM,
    OUTPUT
  );


  pinMode(
    M3_DIR,
    OUTPUT
  );

  pinMode(
    M3_PWM,
    OUTPUT
  );


  pinMode(
    M4_DIR,
    OUTPUT
  );

  pinMode(
    M4_PWM,
    OUTPUT
  );


  // ==========================================================================
  // IR SENSORS
  // ==========================================================================
  //
  // INPUT_PULLUP prevents floating inputs.
  //
  // This is particularly important for the NPN MZ80 outputs.
  //

  pinMode(
    IR_FRONT_LEFT_PIN,
    INPUT_PULLUP
  );

  pinMode(
    IR_FRONT_RIGHT_PIN,
    INPUT_PULLUP
  );

  pinMode(
    IR_LEFT_PIN,
    INPUT_PULLUP
  );

  pinMode(
    IR_RIGHT_PIN,
    INPUT_PULLUP
  );

  pinMode(
    IR_BACK_PIN,
    INPUT_PULLUP
  );


  // ==========================================================================
  // LINE SENSORS
  // ==========================================================================

  pinMode(
    LINE_FRONT_LEFT_PIN,
    INPUT_PULLUP
  );

  pinMode(
    LINE_FRONT_RIGHT_PIN,
    INPUT_PULLUP
  );

  pinMode(
    LINE_BACK_PIN,
    INPUT_PULLUP
  );


  // ==========================================================================
  // START BUTTON
  // ==========================================================================

  pinMode(
    START_BUTTON_PIN,
    INPUT_PULLUP
  );


  // ==========================================================================
  // ULTRASONIC
  // ==========================================================================

  pinMode(
    ULTRASONIC_TRIG_PIN,
    OUTPUT
  );

  pinMode(
    ULTRASONIC_ECHO_PIN,
    INPUT
  );


  digitalWrite(
    ULTRASONIC_TRIG_PIN,
    LOW
  );


  // ==========================================================================
  // INITIAL MOTOR STATE
  // ==========================================================================

  stopMotors();


  delay(500);


  Serial.println(
    "SYSTEM INITIALIZED"
  );


  // ==========================================================================
  // WAIT FOR START
  // ==========================================================================

  waitForStart();

}


// ============================================================================
// LOOP
// ============================================================================

void loop() {

  if (
    !robotStarted
  ) {

    stopMotors();

    return;

  }


  proactiveController();

}